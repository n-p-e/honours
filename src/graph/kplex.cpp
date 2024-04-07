#include "graph/kplex.hpp"

#include "graph/graph.hpp"
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
KPlexDegenResult kPlexDegen(const Graph &g, int64_t k) {
    KPlexDegenResult result = {.kPlex = {}, .ub = 0};
    int64_t size = g.size();

    std::vector<v_id> degrees;
    std::vector<int> removed(size, 0);
    degrees.reserve(size);
    for (v_id i = 0; i < size; i++) { degrees.push_back(g.degreeOf(i)); }
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
        }

        // Line 7-8
        // Upper bound
        int32_t ub = std::min(minDeg + k, size - i + 1);
        if (ub > result.ub) { result.ub = ub; }

        // Line 9
        // Remove current node to start next iteration
        auto &neighbours = g.neighbours(v);
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

KPlexDegenResult kPlexV2(Graph &g, int64_t k, bool twoHop) {
    v_int size = g.size();
    auto initialSolution = kPlexDegen(g, k);
    v_int initialSize = initialSolution.kPlex.size();
    if (initialSize == initialSolution.ub) {
        // result == ub meaning found the best solution
        return initialSolution;
    }
    KPlexDegenResult solution = initialSolution;

    cout << "Initial solution size = " << initialSize << endl;

    // Any vertex with (degree < initialSize - k) definitely won't be in a better answer
    std::vector<int> removed(size, 0);
    int n_removed = 0;
    // add to k-def
    for (v_id i = 0; i < size; i++) {
        if (g.degreeOf(i) <= initialSize - k) {
            removed[i] = 1;
            n_removed++;
        }
    }
    cout << "deleted " << n_removed << " vertices, size " << size << " -> " << (size - n_removed)
         << endl;

    vector<v_id> ordering = degenOrdering(g);
    vector<v_id> degenRank(size, 0); // vertex id -> degeneracy rank from 0 to (n - 1)
    for (v_int i = 0; i < size; i++) { degenRank[ordering[i]] = i; }
    // order neighbours by degeneracy ordering (reversed)
    for (v_id i = 0; i < size; i++) {
        auto neighbours = g.neighbours(i);
        std::sort(neighbours.begin(), neighbours.end(), [&](v_id v1, v_id v2) {
            return degenRank[v1] > degenRank[v2];
        });
        g.setNeighbours(i, neighbours);
    }

    vector<v_id> vMap(size, -1);
    vector<uint8_t> included(size, 0);

    // Generate a subgraph
    for (v_id i = 0; i < size; i++) {
        if (removed[i]) { continue; }
        vector<v_id> vertices;
        vertices.push_back(i);
        included[i] = 1;
        auto &neighbours = g.neighbours(i);
        // Add neighbours and two-hop neighbours to subgraph
        for (v_id j : neighbours) {
            if (degenRank[j] < degenRank[i]) { break; }
            if (!removed[j]) {
                included[j] = 1;
                vertices.push_back(j);
                if (twoHop) {
                    for (v_id k : g.neighbours(j)) {
                        if (degenRank[k] < degenRank[i]) { break; }
                        if (!removed[k]) {
                            included[k] = 1;
                            vertices.push_back(k);
                        }
                    }
                }
            }
        }
        if (vertices.size() <= solution.kPlex.size()) {
            for (auto v : vertices) {
                vMap[v] = -1;
                included[v] = 0;
            }
            continue;
        }

        // Create subgraph
        // Graph subgraph = g.subgraph(vertices);
        Graph subgraph(vertices.size());
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
        auto newSolution = kPlexDegen(subgraph, k);
        if (newSolution.kPlex.size() > solution.kPlex.size()) {
            // Map subgraph vertices back
            vector<v_id> reverseMap(size, -1); // same as vertices, delete
            for (v_id original : vertices) { reverseMap[vMap[original]] = original; }
            for (size_t i = 0; i < newSolution.kPlex.size(); i++) {
                newSolution.kPlex[i] = reverseMap[newSolution.kPlex[i]];
            }
            // cout << "Found better solution" << endl;
            solution = std::move(newSolution);
        }

        // reset:
        for (auto v : vertices) {
            vMap[v] = -1;
            included[v] = 0;
        }
    }

    return solution;
}

bool validateKPlex(const Graph &g, std::vector<v_id> kplex, int k) {
    v_int size = g.size();
    std::vector<int> isInKplex(size, 0);
    for (v_id u : kplex) { isInKplex[u] = 1; }

    for (size_t i = 0; i < kplex.size(); i++) {
        v_id u = kplex[i];
        int numConnections = 0;
        for (v_id v : g.neighbours(u)) {
            if (isInKplex[v]) { numConnections++; }
        }
        if (numConnections < kplex.size() - k) { return false; }
    }
    return true;
}


} // namespace gm
