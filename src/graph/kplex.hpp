#ifndef GM_KPLEX_HPP
#define GM_KPLEX_HPP

#include "graph/graph.hpp"
#include <vector>

namespace gm {

struct KPlexDegenResult {
    std::vector<v_id> kPlex;
    int64_t ub;
};

KPlexDegenResult kPlexDegen(const Graph &g, int64_t k);

KPlexDegenResult kPlexV2(Graph &g, int64_t k, bool twoHop);

bool validateKPlex(const Graph &g, std::vector<v_id> kplex, int k);


} // namespace gm

#endif // GM_KPLEX_HPP
