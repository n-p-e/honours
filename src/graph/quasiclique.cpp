#include "graph/quasiclique.hpp"
#include "graph/graphv2.hpp"
#include "graph/types.hpp"
#include "heap.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using std::vector;

namespace gm {

SubgraphResult quasiCliqueNaive(v2::Graph &graph, double alpha) {
    v_int size = graph.size();
    std::vector<v_int> solution{};
    std::vector<v_int> degrees;
    degrees.reserve(size);
    for (v_id i = 0; i < size; i++) { degrees.push_back(graph.degree(i)); }
    std::vector<v_int> removed(size, 0);

    GraphLinearHeap heap(size, size, degrees);
    for (v_int i = 0; i < size; i++) {
        auto p = heap.popMin();
        v_int v = p.first, minDeg = p.second;
        if (minDeg >= ceil((size - i - 1) * alpha) && size - i > solution.size()) {
            solution = {};
            for (v_id j = 0; j < size; j++) {
                if (!removed[j]) { solution.push_back(j); }
            }
            break;
        }

        for (v_int w : graph.iterNeighbours(v)) {
            if (!removed[w]) {
                degrees[w] -= 1;
                bool res = heap.decrement(w, 1);
                GM_ASSERT(res, "should success");
            }
        }
        removed[v] = 1;
    }
    return {std::move(solution)};
}

SubgraphResult quasiClique(v2::Graph &graph, double alpha, bool twoHop) {
    SubgraphResult solution = quasiCliqueNaive(graph, alpha);
    std::cout << "Initial solution size = " << solution.size << "\n";

    v_int size = graph.size();
    auto ordering = v2::degenOrdering(graph);
    vector<v_id> degenRank(size, 0);
    for (v_int i = 0; i < size; i++) { degenRank[ordering[i]] = i; }
    // order neighbours by degeneracy ordering (reversed)
    for (v_id i = 0; i < size; i++) {
        auto neighbours = graph.iterNeighbours(i);
        std::sort(neighbours.begin(), neighbours.end(), [&](v_id v1, v_id v2) {
            return degenRank[v1] > degenRank[v2];
        });
    }

    vector<v_id> vMap(size, -1);
    vector<int> included(size, 0);
    for (v_id u = 0; u < size; u++) {
        // prove if we should take = here
        if (graph.degree(u) <= ceil((solution.size - 1) * alpha)) { continue; }
        vector<v_id> vertices;
        vertices.push_back(u);
        included[u] = 1;
        auto neighbours = graph.iterNeighbours(u);
        // Add neighbours and two-hop neighbours to subgraph
        for (v_id v : neighbours) {
            if (degenRank[v] < degenRank[u]) { break; }
            if (graph.degree(v) <= ceil((solution.size - 1) * alpha)) { continue; }

            if (!included[v]) {
                included[v] = 1;
                vertices.push_back(v);
            }
            if (twoHop) {
                for (v_int w : graph.iterNeighbours(v)) {
                    if (degenRank[w] < degenRank[u]) { break; }
                    if (graph.degree(w) <= ceil((solution.size - 1) * alpha)) { continue; }

                    if (!included[w]) {
                        included[w] = 1;
                        vertices.push_back(w);
                    }
                }
            }
        }
        if (vertices.size() > solution.subgraph.size()) {
            auto subgraph = v2::subgraphDegen(graph, vertices, degenRank.data());

            auto newSolution = quasiCliqueNaive(subgraph, alpha);
            if (newSolution.size > solution.subgraph.size()) {
                for (size_t i = 0; i < newSolution.size; i++) {
                    newSolution.subgraph[i] = vertices[newSolution.subgraph[i]];
                }
                std::cout << "Found better solution " << newSolution.size << "\n";
                solution = std::move(newSolution);
            }
        }

        for (auto v : vertices) {
            vMap[v] = -1;
            included[v] = 0;
        }
    }
    return solution;
}

bool validateQuasiClique(v2::Graph &graph, const std::vector<v_id> &quasiClique, double alpha) {
    auto size = graph.size();
    auto threshold = std::ceil(alpha * (quasiClique.size() - 1));
    std::vector<uint8_t> included(size, 0);
    for (v_id u : quasiClique) { included[u] = 1; }

    for (auto u : quasiClique) {
        v_int degree = 0;
        for (auto v : graph.iterNeighbours(u)) {
            if (included[v]) { degree++; }
        }
        if (degree < threshold) { return false; }
    }
    return true;
}

} // namespace gm
