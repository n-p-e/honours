#include "graph/kplex.hpp"

#include "graph/graph.hpp"
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
    std::unordered_set<v_id> removed;
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
                if (!removed.contains(j)) { result.kPlex.push_back(j); }
            }
        }

        // Line 7-8
        // Upper bound
        int32_t ub = std::min(minDeg + k, size - i + 1);
        if (ub > result.ub) { result.ub = ub; }

        // Line 9
        // Remove current node to start next iteration
        auto neighbours = g.neighbours(v);
        for (v_id w : neighbours) {
            if (!removed.contains(w)) {
                degrees[w] -= 1;
                auto res = heap.decrement(w, 1);
                GM_ASSERT(res, "should success");
            }
        }
        removed.insert(v);
    }
    return result;
}

KPlexDegenResult kPlexV2(const Graph &g, int64_t k) {
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
    std::unordered_set<v_id> removed;
    for (v_id i = 0; i < size; i++) {
        if (g.degreeOf(i) < initialSize - k) { removed.insert(i); }
    }
    cout << "deleted " << removed.size() << " vertices, size " << size << " -> "
         << (size - removed.size()) << endl;

    vector<v_id> ordering = degenOrdering(g);
    vector<v_id> degenRank(size, 0);
    for (v_int i = 0; i < size; i++) { degenRank[ordering[i]] = i; }

    // Generate a subgraph
    for (v_id i = 0; i < size; i++) {
        if (removed.contains(i)) { continue; }
        vector<v_id> vertices;
        vertices.push_back(i);
        auto &neighbours = g.neighbours(i);
        for (v_id j : neighbours) {
            if (!removed.contains(j) && degenRank[j] > degenRank[i]) { vertices.push_back(j); }
        }
        if (vertices.size() < initialSize) {
            continue;
        }
        Graph subgraph = g.subgraph(vertices);
        cout << "calculating subgraph (size=" << subgraph.size() << ")" << endl;
        auto newSolution = kPlexDegen(subgraph, k);
        if (newSolution.kPlex.size() > solution.kPlex.size()) {
            cout << "Found better solution" << endl;
            solution = std::move(newSolution);
        }
    }

    return solution;
}


} // namespace gm
