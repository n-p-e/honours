#ifndef GM_QUASICLIQUE_HPP
#define GM_QUASICLIQUE_HPP

#include <vector>

#include "graph/graphv2.hpp"
#include "graph/types.hpp"

namespace gm {

SubgraphResult quasiClique(v2::Graph &graph, double alpha);

bool validateQuasiClique(v2::Graph &graph, const std::vector<v_id> &quasiClique, double alpha);

} // namespace gm

#endif
