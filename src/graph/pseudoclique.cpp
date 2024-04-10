#include "graph/pseudoclique.hpp"

#include "graph/graphv2.hpp"
#include "graph/types.hpp"

#include <cmath>
#include <vector>

using std::vector;

namespace gm {

SubgraphResult pseudoCliqueNaive(v2::Graph &graph, double alpha) {
    v_int size = graph.size();
    auto ordering = v2::degenOrdering(graph);
    std::vector<v_int> solution{};
    for (v_int i = size - 1; i >= 0; i--) {
        solution.push_back(ordering[i]);
        if (!validatePseudoClique(graph, solution, alpha)) {
            solution.pop_back();
            return {solution, v_int(solution.size())};
        }
    }
    return {solution, v_int(solution.size())};
}

SubgraphResult pseudoClique(v2::Graph &graph, double alpha, bool twoHop) {
    SubgraphResult solution = {};

    v_int size = graph.size();
    auto ordering = v2::degenOrdering(graph);
    vector<v_id> degenRank(size, 0);
    vector<uint8_t> removed(size, 0);
    for (v_int i = 0; i < size; i++) { degenRank[ordering[i]] = i; }
    // order neighbours by degeneracy ordering (reversed)
    for (v_id i = 0; i < size; i++) {
        auto neighbours = graph.iterNeighbours(i);
        std::sort(neighbours.begin(), neighbours.end(), [&](v_id v1, v_id v2) {
            return degenRank[v1] > degenRank[v2];
        });
    }

    // Generate a subgraph
    vector<uint8_t> included(size, 0);
    for (v_id i = 0; i < size; i++) {
        if (removed[i] || graph.degree(i) <= solution.size) { continue; }
        vector<v_id> vertices;
        vertices.push_back(i);
        included[i] = 1;
        auto neighbours = graph.iterNeighbours(i);
        // Add neighbours and two-hop neighbours to subgraph
        for (v_id j : neighbours) {
            if (!removed[j]) {
                if (degenRank[j] < degenRank[i]) { break; }
                vertices.push_back(j);
                included[j] = 1;
            }
            if (twoHop) {
                for (v_int k : graph.iterNeighbours(j)) {
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
            vector<v_id> vMap;

            auto subgraph = v2::subgraphDegen(graph, vertices, degenRank.data());

            auto newSolution = pseudoCliqueNaive(subgraph, alpha);
            if (newSolution.size > solution.size) {
                for (size_t i = 0; i < newSolution.size; i++) {
                    newSolution.subgraph[i] = vertices[newSolution.subgraph[i]];
                }
                solution = std::move(newSolution);
            }
        }
        for (auto v : vertices) { included[v] = 0; }
    }
    return solution;
}

bool validatePseudoClique(v2::Graph &graph, const std::vector<v_id> &pseudoClique, double alpha) {
    auto size = graph.size();
    auto threshold = std::ceil(0.5 * alpha * pseudoClique.size() * (pseudoClique.size() - 1));
    std::vector<uint8_t> included(size, 0);
    for (v_id u : pseudoClique) { included[u] = 1; }

    v_int nEdges = 0;
    for (auto u : pseudoClique) {
        for (auto v : graph.iterNeighbours(u)) {
            if (included[v]) { nEdges++; }
        }
    }
    return nEdges >= threshold * 2;
}

} // namespace gm
