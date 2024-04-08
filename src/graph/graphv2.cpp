#include "graphv2.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_set>
#include <utility>
#include <vector>

#include "graph/types.hpp"
#include "heap.hpp"

using namespace std;

namespace gm::v2 {

namespace fs = std::filesystem;

GraphV2::GraphV2(v_int n, v_int m) : n(n), m(m), off(new v_int[n + 1]), e(new v_int[2 * m]) {
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

GraphV2 readGraphBinary(std::string path) {
    std::cout << "[readGraphBinary] reading using b_degree.bin and b_adj.bin files\n";
    std::string degreesPath = path + "/b_degree.bin";
    std::string edgesPath = path + "/b_adj.bin";
    FILE *fp = fopen(degreesPath.c_str(), "rb");
    size_t ret = 0;
    v_int tt;
    v_int nm[2];
    (ret = fread(&tt, sizeof(tt), 1, fp));
    GM_ASSERT(ret == 1, ("readGraphBinary"));
    GM_ASSERT(tt == sizeof(v_int), ("readGraphBinary"));
    ret = fread(&nm, sizeof(nm[0]), 2, fp);
    GM_ASSERT(ret == 2, ("readGraphBinary"));

    v_int n = nm[0], m = nm[1];
    v_int *degrees = new v_int[n + 1];
    v_int *edges = new v_int[m];

    std::vector<v_int> adjs(m, 0);
    ret = fread(degrees, sizeof(degrees[0]), n, fp);
    GM_ASSERT(ret == n, ("readGraphBinary"));
    fclose(fp);

    FILE *fpEdges = fopen(edgesPath.c_str(), "rb");
    ret = fread(edges, sizeof(edges[0]), m, fpEdges);
    GM_ASSERT(ret == m, ("readGraphBinary"));
    fclose(fpEdges);

    v_int *start = edges;
    v_int *dest = edges;
    v_int real_m = 0; // m after removing duplicates
    // in-place remove duplicates and self-loops
    for (v_int i = 0; i < n; i++) {
        v_int *end = start + degrees[i];
        v_int *destStart = dest;
        std::sort(start, end);
        for (v_int *v = start; v != end; v++) {
            if ((*v != i) && (v == start || *v != *(v - 1))) {
                *dest = *v;
                dest++;
            }
        }
        // turn degrees into offset (prefix sum)
        degrees[i] = destStart - edges;
        real_m += dest - destStart;
        start = end;
    }
    degrees[n] = real_m;
    if (m != real_m) {
        std::cout << std::format(
            "{} -> {} edges after removing duplicates and self-loops\n", m / 2, real_m / 2);
    }
    GraphV2 g{n, real_m / 2, degrees, edges};
    return g;
}

GraphV2 GraphV2::readFromFile(std::string path) {
    if (fs::is_directory(path)) { return readGraphBinary(path); }
    cerr << format("[Graph::readFromFile] reading from {}\n", path);
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
    static vector<v_int> vMap(this->size(), -1); // optimize
    // std::sort(vertices.begin(), vertices.end());
    v_int nextId = 0;
    for (v_int v : vertices) {
        vMap[v] = nextId;
        nextId++;
    }
    static std::vector<pair<v_int, v_int>> edges;
    edges.clear();
    for (v_int u : vertices) {
        for (v_int v : this->iterNeighbours(u)) {
            if (vMap[v] >= 0) {
                // reverse will also be pushed
                // we guarantee all edges starting with u is consecutive
                edges.push_back(make_pair(vMap[u], vMap[v]));
                // TODO: break if degen
            }
        }
    }

    // std::sort(edges.begin(), edges.end());

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

    // if (vMapOut) { *vMapOut = std::move(vMap); }
    for (v_int u : vertices) { vMap[u] = -1; }

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

    for (v_id i = 0; i < g.size(); i++) { degrees.push_back(g.degree(i)); }

    GraphLinearHeap heap(g.size(), g.size() + 1, degrees);
    for (v_id i = 0; i < g.size(); i++) {
        auto smallestDeg = heap.popMin();
        v_id u = smallestDeg.first;
        for (v_id *vp = g.edges(u); vp != g.edgesEnd(u); vp++) { heap.decrement(*vp, 1); }
        result.push_back(u);
    }
    return result;
}

GraphV2 subgraphDegen(GraphV2 &g, std::vector<v_int> &vertices, v_int *degenRank) {
    // Map from old vertex id -> new vertex id
    static vector<v_int> vMap(g.size(), -1);
    v_int size = vertices.size();
    // std::sort(vertices.begin(), vertices.end());
    v_int nextId = 0;
    for (v_int v : vertices) {
        vMap[v] = nextId;
        nextId++;
    }
    static std::vector<pair<v_int, v_int>> edges;
    edges.clear();
    v_int *degrees = new v_int[size]{0};
    v_int *offsets = new v_int[size + 1]{0};

    for (v_int u : vertices) {
        for (v_int v : g.iterNeighbours(u)) {
            if (vMap[v] >= 0) {
                if (degenRank[v] < degenRank[u]) { break; }

                // reverse will also be pushed
                edges.push_back(make_pair(vMap[u], vMap[v]));
                edges.push_back(make_pair(vMap[v], vMap[u]));
                degrees[vMap[u]]++;
                degrees[vMap[v]]++;
            }
        }
    }
    v_int *adj = new v_int[edges.size()];

    for (v_int i = 0; i < size; i++) { offsets[i + 1] = offsets[i] + degrees[i]; }
    for (const auto &e : edges) {
        v_int u = e.first, v = e.second;
        adj[offsets[u]] = v;
        offsets[u] = offsets[u] + 1;
    }
    offsets[0] = 0;
    for (v_int i = 0; i < size; i++) { offsets[i + 1] = offsets[i] + degrees[i]; }

    GraphV2 sub{size, (v_int) edges.size(), offsets, adj};
    // if (vMapOut) { *vMapOut = std::move(vMap); }
    for (auto u : vertices) { vMap[u] = -1; }

    return sub;
}


} // namespace gm::v2
