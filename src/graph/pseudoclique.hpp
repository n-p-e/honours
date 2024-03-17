#ifndef GM_PSEUDOCLIQUE_HPP
#define GM_PSEUDOCLIQUE_HPP

#include <vector>

#include "graph/graphv2.hpp"
#include "graph/types.hpp"

namespace gm {

SubgraphResult pseudoCliqueNaive(v2::Graph &graph, double alpha);
SubgraphResult pseudoClique(v2::Graph &graph, double alpha);

bool validatePseudoClique(v2::Graph &graph, const std::vector<v_id> &pseudoClique, double alpha);

} // namespace gm

#endif
