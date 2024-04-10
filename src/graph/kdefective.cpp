#include "graph/kdefective.hpp"
#include "graph/graph.hpp"
#include "graph/graphv2.hpp"
#include "graph/types.hpp"
#include <iostream>
#include <vector>

using namespace std;

namespace gm {

kDefResult kDefNaive(Graph &g, v_int k) {
    kDefResult result{};
    v_int size = g.size();
    auto ordering = degenOrdering(g);
    std::vector<v_id> solution{};
    for (v_int idx = size - 1; idx >= 0; idx--) {
        v_id u = ordering[idx];
        solution.push_back(u);
        if (!checkKDef(g, solution, k)) {
            solution.pop_back();
            break;
        }
    }
    result.kDefective = solution;
    result.size = solution.size();
    return result;
}

kDefResult kDefNaiveV2(v2::GraphV2 &g, v_int k) {
    kDefResult result{};
    v_int size = g.size();
    auto ordering = degenOrdering(g);
    std::vector<v_id> solution{};
    for (v_int idx = size - 1; idx >= 0; idx--) {
        v_id u = ordering[idx];
        solution.push_back(u);
        if (!checkKDefV2(g, solution, k)) {
            solution.pop_back();
            break;
        }
    }
    result.kDefective = std::move(solution);
    result.size = solution.size();
    return result;
}

// not used, only v2 is used for now
kDefResult kDefDegen(Graph &g, v_int k) {
    kDefResult solution{};
    v_int size = g.size();
    vector<v_id> ordering = degenOrdering(g);
    vector<v_id> degenRank(size, 0); // vertex id -> degeneracy rank from 0 to (n - 1)
    std::vector<int> removed(size, 0);

    for (v_int i = 0; i < size; i++) { degenRank[ordering[i]] = i; }
    // order neighbours by degeneracy ordering (reversed)
    for (v_id i = 0; i < size; i++) {
        auto neighbours = g.neighbours(i);
        std::sort(neighbours.begin(), neighbours.end(), [&](v_id v1, v_id v2) {
            return degenRank[v1] > degenRank[v2];
        });
        g.setNeighbours(i, neighbours);
    }

    // Generate a subgraph
    for (v_id i = 0; i < size; i++) {
        if (removed[i]) { continue; }
        vector<v_id> vertices;
        vector<int> included(size, 0);
        included[i] = 1;
        auto &neighbours = g.neighbours(i);
        // Add neighbours and two-hop neighbours to subgraph
        for (v_id j : neighbours) {
            if (!removed[j]) {
                if (degenRank[j] < degenRank[i]) { break; }
                included[j] = 1;
            }
        }
        for (v_id j = 0; j < size; j++) {
            if (included[j]) { vertices.push_back(j); }
        }
        // if (vertices.size() < initialSize) { continue; }

        // Create subgraph
        // Graph subgraph = g.subgraph(vertices);
        Graph subgraph(vertices.size());
        vector<v_id> vMap(size, -1);
        v_id nextId = 0;
        for (v_id v : vertices) {
            vMap[v] = nextId;
            nextId++;
        }
        for (v_id u : vertices) {
            for (v_id v : g.neighbours(u)) {
                // Optimize with degenRank as the neighbours arrays are ordered
                if (degenRank[u] < degenRank[v]) {
                    if (vMap[v] >= 0) {
                        // cerr << "addEdge " << vMap[u] << " " << vMap[v] << endl;
                        subgraph.addEdge(vMap[u], vMap[v]);
                    }
                } else {
                    break;
                }
            }
        }

        // cout << "calculating subgraph (size=" << subgraph.size() << ")" << endl;
        auto newSolution = kDefNaive(subgraph, k);
        if (newSolution.kDefective.size() > solution.kDefective.size()) {
            for (size_t i = 0; i < newSolution.kDefective.size(); i++) {
                newSolution.kDefective[i] = vertices[newSolution.kDefective[i]];
            }
            // cout << "Found better solution" << endl;
            solution = std::move(newSolution);
        }
    }
    return solution;
}

kDefResult kDefDegenV2(v2::GraphV2 &g, v_int k, bool twoHop) {
    kDefResult solution{};
    v_int size = g.size();
    vector<v_id> ordering = degenOrdering(g);
    vector<v_id> degenRank(size, 0); // vertex id -> degeneracy rank from 0 to (n - 1)

    for (v_int i = 0; i < size; i++) { degenRank[ordering[i]] = i; }
    // order neighbours by degeneracy ordering (reversed)
    for (v_id i = 0; i < size; i++) {
        auto neighbours = g.iterNeighbours(i);
        std::sort(neighbours.begin(), neighbours.end(), [&](v_id v1, v_id v2) {
            return degenRank[v1] > degenRank[v2];
        });
    }

    vector<uint8_t> included(size, 0);
    // Generate a subgraph
    // parallelise w/ openmp
    for (v_id i = 0; i < size; i++) {
        if (g.degree(i) <= solution.size) { continue; }
        vector<v_id> vertices;
        vertices.push_back(i);
        included[i] = 1;
        auto neighbours = g.iterNeighbours(i);
        // Add neighbours and two-hop neighbours to subgraph
        for (v_id j : neighbours) {
            if (degenRank[j] < degenRank[i]) { break; }
            if (!included[j]) {
                included[j] = 1;
                vertices.push_back(j);
            }
            if (twoHop) {
                for (v_int k : g.iterNeighbours(j)) {
                    if (degenRank[k] < degenRank[i]) { break; }
                    if (!included[k]) {
                        included[k] = 1;
                        vertices.push_back(k);
                    }
                }
            }
        }

        if (vertices.size() > solution.size) {
            // Create subgraph
            auto subgraph = v2::subgraphDegen(g, vertices, degenRank.data());

            auto newSolution = kDefNaiveV2(subgraph, k);
            if (newSolution.kDefective.size() > solution.kDefective.size()) {
                for (size_t i = 0; i < newSolution.kDefective.size(); i++) {
                    newSolution.kDefective[i] = vertices[newSolution.kDefective[i]];
                }
                cout << "Found better solution of size " << newSolution.size << endl;
                solution = std::move(newSolution);
            }
        }
        for (auto v : vertices) { included[v] = 0; }
    }
    return solution;
}

bool checkKDef(Graph &g, const std::vector<v_id> &vs, v_int k) {
    v_int size = g.size();
    v_int target = vs.size() * (vs.size() - 1) / 2;
    std::vector<uint8_t> included(size, 0);
    v_int count = 0;
    for (v_id u : vs) { included[u] = 1; }
    for (v_id u : vs) {
        for (v_id v : g.neighbours(u)) {
            if (u < v && included[v]) { count++; }
        }
    }
    if (target - count > k) { return false; }
    return true;
}

bool checkKDefV2(v2::GraphV2 &g, const std::vector<v_id> &vs, v_int k) {
    v_int size = g.size();
    int64_t target = int64_t(vs.size()) * (vs.size() - 1) / 2;
    std::vector<uint8_t> included(size, 0);
    v_int count = 0;
    for (v_id u : vs) { included[u] = 1; }
    for (v_id u : vs) {
        for (v_id v : g.iterNeighbours(u)) {
            if (u < v && included[v]) { count++; }
        }
    }
    if (target - count > k) { return false; }
    return true;
}

} // namespace gm
