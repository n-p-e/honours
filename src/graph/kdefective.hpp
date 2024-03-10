#ifndef GM_KDEFECTIVE_HPP
#define GM_KDEFECTIVE_HPP

#include "graph/graph.hpp"
#include "graph/graphv2.hpp"
#include "graph/types.hpp"
#include <vector>

namespace gm {

struct kDefResult {
    std::vector<v_id> kDefective;
    v_int size;
};

kDefResult kDefNaive(Graph &g, v_int k);
kDefResult kDefNaive(v2::GraphV2 &g, v_int k);
kDefResult kDefDegen(Graph &g, v_int k);
kDefResult kDefDegen(v2::GraphV2 &g, v_int k);

bool checkKDef(Graph &g, const std::vector<v_id> &vs, v_int k);
bool checkKDef(v2::GraphV2 &g, const std::vector<v_id> &vs, v_int k);

} // namespace gm

#endif // GM_KDEFECTIVE_HPP
