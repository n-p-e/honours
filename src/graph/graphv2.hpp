#ifndef GM_GRAPHV2_HPP
#define GM_GRAPHV2_HPP

#include "graph/types.hpp"
#include <ostream>
#include <string>

namespace gm::v2 {

class GraphV2 {
    class NeighboursRange {
    public:
        inline auto begin() {
            return begin_;
        }
        inline auto end() {
            return end_;
        }

        NeighboursRange(v_int *begin, v_int *end) : begin_(begin), end_(end) {}

    private:
        v_int *begin_;
        v_int *end_;
    };

public:
    GraphV2(v_int n, v_int m);
    GraphV2(const GraphV2 &) = delete;
    GraphV2 &operator=(const GraphV2 &) = delete;
    GraphV2(GraphV2 &&other);
    ~GraphV2();
    static GraphV2 readFromFile(std::string path);


    inline v_int size() const {
        return n;
    }
    inline v_int nEdges() const {
        return m;
    }
    inline NeighboursRange iterNeighbours(v_int u) {
        return NeighboursRange{edges(u), edgesEnd(u)};
    }
    inline v_int *edges(v_int u) {
        return e + off[u];
    }
    inline v_int *edgesEnd(v_int u) {
        return e + off[u + 1];
    }
    inline v_int degree(v_int u) {
        return off[u + 1] - off[u];
    }
    GraphV2 subgraph(const std::vector<v_int> &vertices, std::vector<v_int> *vMapOut = nullptr);
    friend std::ostream &operator<<(std::ostream &os, const GraphV2 &g);

private:
    v_int *e;
    v_int *off;
    v_int n, m;
    inline v_int eSize() {
        return 2 * m;
    }
    inline v_int offSize() {
        return n;
    }
};
using Graph = GraphV2;

std::vector<v_id> degenOrdering(GraphV2 &g);

} // namespace gm::v2

#endif