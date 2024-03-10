#include "graph/quasiclique.hpp"
#include "graph/graphv2.hpp"
#include "graph/types.hpp"

#include <cmath>
#include <vector>

namespace gm {

QuasiCliqueResult quasiCliqueNaive(v2::Graph &graph, double alpha) {
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

QuasiCliqueResult quasiClique(v2::Graph &graph, double alpha) {
    return quasiCliqueNaive(graph, alpha);
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
