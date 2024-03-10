#include "graphv2.hpp"

#include "graph/types.hpp"
#include "heap.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

namespace gm::v2 {

GraphV2::GraphV2(v_int n, v_int m) : e(new v_int[2 * m]), off(new v_int[n + 1]), n(n), m(m) {
    off[n] = 2 * m;
}
GraphV2::GraphV2(GraphV2 &&other) {
    e = other.e;
    off = other.off;
    other.e = other.off = nullptr;
}

GraphV2::~GraphV2() {
    delete[] e;
    delete[] off;
}

GraphV2 GraphV2::readFromFile(std::string path) {
    std::ifstream ifs{path, std::ifstream::in};
    v_int n, m;
    ifs >> n >> m;

    GraphV2 g{n, m};
    std::vector<std::pair<v_int, v_int>> edges;

    for (v_int i = 0; i < m; i++) {
        v_id u, v;
        ifs >> u >> v;
        edges.push_back(make_pair(u, v));
        edges.push_back(make_pair(v, u));
    }
    std::sort(edges.begin(), edges.end());

    // Init offset to -1
    memset(g.off, -1, sizeof(v_int) * n);

    for (auto i = 0; i < edges.size(); i++) {
        int u = edges[i].first;
        int v = edges[i].second;
        if (g.off[u] == -1) { g.off[u] = i; }
        g.e[i] = v;
    }
    for (auto i = 0; i < g.size(); i++) {
        if (g.off[i] == -1) { g.off[i] = (i == 0 ? 0 : g.off[i - 1]); }
    }
    g.off[g.size()] = g.eSize();
    return g;
}

GraphV2
GraphV2::subgraph(const std::vector<v_int> &vertices, std::vector<v_int> *vMapOut /* = nullptr */) {
    // Map from old vertex id -> new vertex id
    vector<v_int> vMap(this->size(), -1);
    v_int nextId = 0;
    for (v_int v : vertices) {
        vMap[v] = nextId;
        nextId++;
    }
    std::vector<pair<v_int, v_int>> edges;
    for (v_int u : vertices) {
        for (v_int v : this->iterNeighbours(u)) {
            if (vMap[v] >= 0 && u < v) { // avoid duplicate edges
                edges.push_back(make_pair(vMap[u], vMap[v]));
                edges.push_back(make_pair(vMap[v], vMap[u]));
            }
        }
    }

    std::sort(edges.begin(), edges.end());

    GraphV2 g{v_int(vertices.size()), v_int(edges.size() / 2)};
    memset(g.off, -1, sizeof(v_int) * g.size());

    v_int i = 0;
    for (const auto &edge : edges) {
        v_int u = edge.first;
        v_int v = edge.second;
        if (g.off[u] == -1) { g.off[u] = i; }
        g.e[i] = v;
        i++;
    }
    for (auto i = 0; i < g.size(); i++) {
        if (g.off[i] == -1) { g.off[i] = (i == 0 ? 0 : g.off[i - 1]); }
    }
    g.off[g.size()] = g.eSize();

    if (vMapOut) { *vMapOut = std::move(vMap); }

    return g;
}

std::ostream &operator<<(std::ostream &os, const GraphV2 &graph) {
    return os << "Graph{size=" << graph.size() << ",edges=" << graph.nEdges() << "}";
}

std::vector<v_id> degenOrdering(GraphV2 &g) {
    vector<v_id> result;
    vector<v_id> degrees;
    result.reserve(g.size());
    degrees.reserve(g.size());

    for (v_id i = 0; i < g.size(); i++) {
        degrees.push_back(g.degree(i));
    }

    GraphLinearHeap heap(g.size(), g.size() + 1, degrees);
    for (v_id i = 0; i < g.size(); i++) {
        auto smallestDeg = heap.popMin();
        v_id u = smallestDeg.first;
        for (v_id *vp = g.edges(u); vp != g.edgesEnd(u); vp++) {
            heap.decrement(*vp, 1);
        }
        result.push_back(u);
    }
    return result;
}


} // namespace gm::v2