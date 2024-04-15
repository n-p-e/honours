#include "graph/kplex.hpp"

#include "graph/graphv2.hpp"
#include "graph/types.hpp"
#include "heap.hpp"
#include "util.hpp"
#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace gm {

using namespace std;

// Algorithm 2: kPlex-Degen
KPlexDegenResult kPlexDegen(v2::Graph &g, int64_t k) {
    KPlexDegenResult result = {.kPlex = {}, .ub = 0};
    int64_t size = g.size();

    std::vector<v_id> degrees;
    std::vector<int> removed(size, 0);
    degrees.reserve(size);
    for (v_id i = 0; i < size; i++) { degrees.push_back(g.degree(i)); }
    GraphLinearHeap heap(g.size(), g.size(), degrees);

    for (v_id i = 0; i < size; i++) {
        // Line 4
        // Find node with smallest degree
        auto p = heap.popMin();
        v_id v = p.first;
        v_id minDeg = p.second;
        // Line 5-6
        // All nodes that's not removed form a k-plex
        if (minDeg + k >= size - i + 1 && size - i + 1 > result.kPlex.size()) {
            result.kPlex = {};
            for (v_id j = 0; j < size; j++) {
                if (!removed[j]) { result.kPlex.push_back(j); }
            }
            break;
        }

        // Line 7-8
        // Upper bound
        int32_t ub = std::min(minDeg + k, size - i + 1);
        if (ub > result.ub) { result.ub = ub; }

        // Line 9
        // Remove current node to start next iteration
        auto neighbours = g.iterNeighbours(v);
        for (v_id w : neighbours) {
            if (!removed[w]) {
                degrees[w] -= 1;
                auto res = heap.decrement(w, 1);
                GM_ASSERT(res, "should success");
            }
        }
        removed[v] = 1;
    }
    return result;
}

KPlexDegenResult kPlexV2(v2::GraphV2 &g, int64_t k, bool twoHop) {
    v_int size = g.size();
    auto initialSolution = kPlexDegen(g, k);
    v_int initialSize = initialSolution.kPlex.size();
    if (initialSize == initialSolution.ub) {
        // result == ub meaning found the best solution
        return initialSolution;
    }
    KPlexDegenResult solution = initialSolution;

    cout << "Initial solution size = " << initialSize << endl;

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

    // vector<v_id> vMap(size, -1);
    vector<uint8_t> included(size, 0);

    // Generate a subgraph
    for (v_id u = 0; u < size; u++) {
        // Any vertex with (degree < initialSize - k) definitely won't be in a better answer
        if (g.degree(u) <= solution.kPlex.size() - k) { continue; }
        vector<v_id> vertices;
        vertices.push_back(u);
        included[u] = 1;
        auto neighbours = g.iterNeighbours(u);
        // Add neighbours and two-hop neighbours to subgraph
        for (v_id v : neighbours) {
            if (degenRank[v] < degenRank[u]) { break; }
            if (g.degree(v) <= solution.kPlex.size() - k) { continue; }

            if (!included[v]) {
                included[v] = 1;
                vertices.push_back(v);
            }
            if (twoHop) {
                for (v_id w : g.iterNeighbours(v)) {
                    if (degenRank[w] < degenRank[u]) { break; }
                    if (g.degree(w) <= solution.kPlex.size() - k) { continue; }
                    if (!included[w]) {
                        included[w] = 1;
                        vertices.push_back(w);
                    }
                }
            }
        }
        if (vertices.size() <= solution.kPlex.size()) {
            for (auto v : vertices) {
                // vMap[v] = -1;
                included[v] = 0;
            }
            continue;
        }

        // Create subgraph
        // v2::Graph subgraph = g.subgraph(vertices);
        v2::Graph subgraph = v2::subgraphDegen(g, vertices, degenRank.data());
        // auto subgraph = v2::subgraphDegen(g, vertices, &vMap, degenRank.data());
        // v_id nextId = 0;
        // for (v_id v : vertices) {
        //     vMap[v] = nextId;
        //     nextId++;
        // }

        // cout << "calculating subgraph (size=" << subgraph.size() << ")" << endl;
        auto newSolution = kPlexDegen(subgraph, k);
        if (newSolution.kPlex.size() > solution.kPlex.size()) {
            // Map subgraph vertices back
            for (size_t i = 0; i < newSolution.kPlex.size(); i++) {
                newSolution.kPlex[i] = vertices[newSolution.kPlex[i]];
            }
            // cout << "Found better solution size = " << newSolution.kPlex.size() << endl;
            solution = std::move(newSolution);
        }

        // reset:
        for (auto v : vertices) {
            // vMap[v] = -1;
            included[v] = 0;
        }
    }

    return solution;
}

bool validateKPlex(v2::Graph &g, std::vector<v_id> kplex, int k) {
    v_int size = g.size();
    std::vector<int> isInKplex(size, 0);
    for (v_id u : kplex) { isInKplex[u] = 1; }

    for (size_t i = 0; i < kplex.size(); i++) {
        v_id u = kplex[i];
        int numConnections = 0;
        for (v_id v : g.iterNeighbours(u)) {
            if (isInKplex[v]) { numConnections++; }
        }
        if (numConnections < kplex.size() - k) { return false; }
    }
    return true;
}


} // namespace gm
