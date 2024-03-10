#ifndef GM_TYPES_HPP
#define GM_TYPES_HPP

#include <cstdint>
#include <vector>

namespace gm {

// Use this type for stuff like count of vertices
using v_int = int32_t;
// Vertex ID
using v_id = v_int;

using e_int = int64_t;

struct SubgraphResult {
    std::vector<v_id> subgraph;
    v_int size;
};

} // namespace gm

#endif // GM_TYPES_HPP
