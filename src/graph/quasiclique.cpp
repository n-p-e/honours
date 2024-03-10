#include "graph/quasiclique.hpp"
#include "graph/graphv2.hpp"
#include "graph/types.hpp"

#include <cmath>
#include <vector>

using std::vector;

namespace gm {

SubgraphResult quasiCliqueNaive(v2::Graph &graph, double alpha) {
    v_int size = graph.size();
    auto ordering = v2::degenOrdering(graph);
    std::vector<v_int> solution{};
    for (v_int i = size - 1; i >= 0; i--) {
        solution.push_back(ordering[i]);
        if (!validateQuasiClique(graph, solution, alpha)) {
            solution.pop_back();
            return {solution, v_int(solution.size())};
        }
    }
    return {solution, v_int(solution.size())};
}

SubgraphResult quasiClique(v2::Graph &graph, double alpha) {
    SubgraphResult solution = quasiCliqueNaive(graph, alpha);

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

    // Generate a subgraph
    for (v_id i = 0; i < size; i++) {
        if (graph.degree(i) <= solution.size) { continue; }
        vector<v_id> vertices;
        vector<int> included(size, 0);
        included[i] = 1;
        auto neighbours = graph.iterNeighbours(i);
        // Add neighbours and two-hop neighbours to subgraph
        for (v_id j : neighbours) {
            if (degenRank[j] < degenRank[i]) { break; }
            included[j] = 1;
        }
        for (v_id j = 0; j < size; j++) {
            if (included[j]) { vertices.push_back(j); }
        }
        if (vertices.size() < solution.size) { continue; }

        // Create subgraph
        vector<v_id> vMap;

        auto subgraph = graph.subgraph(vertices, &vMap);

        auto newSolution = quasiCliqueNaive(subgraph, alpha);
        if (newSolution.subgraph.size() > solution.subgraph.size()) {
            // Map subgraph vertices back
            vector<v_id> reverseMap(size, -1);
            for (v_id original : vertices) { reverseMap[vMap[original]] = original; }
            for (size_t i = 0; i < newSolution.subgraph.size(); i++) {
                newSolution.subgraph[i] = reverseMap[newSolution.subgraph[i]];
            }
            // cout << "Found better solution" << endl;
            solution = std::move(newSolution);
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
