#include "graph/graph.hpp"
#include "heap.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <numeric>
#include <unordered_set>
#include <vector>

namespace gm {
using namespace std;

// -- Graph
Graph readGraph(const std::string &path) {
    ifstream ifs{path, ifstream::in};
    int64_t n, m;
    ifs >> n >> m;
    Graph graph{n};
    for (int64_t i = 0; i < m; i++) {
        v_id u, v;
        ifs >> u >> v;
        graph.addEdge(u, v);
    }
    return graph;
}

Graph::Graph(int64_t size) : size_(size), v({}), adjs({}) {
    adjs.resize(size);
    // not useful for now
    // for (int64_t i = 0; i < size; i++) { v.emplace_back(this, i); }
}
uint64_t Graph::size() const {
    return size_;
}
int64_t Graph::degreeOf(v_id vertex) const {
    return adjs[vertex].size();
}
const std::vector<v_id> &Graph::neighbours(v_id vertex) const {
    return adjs[vertex];
}
void Graph::addEdge(v_id v1, v_id v2) {
    adjs[v1].push_back(v2);
    adjs[v2].push_back(v1);
}

Graph Graph::subgraph(const std::vector<v_id> &vertices) const {
    Graph result(this->size());
    unordered_set<v_id> vertSet(vertices.cbegin(), vertices.cend());
    for (v_id u : vertices) {
        for (v_id v : this->neighbours(u)) {
            if (vertSet.contains(v) && u < v) { // avoid duplicate edges
                result.addEdge(u, v);
            }
        }
    }
    return result;
}

std::vector<v_id> degenOrdering(const Graph &g) {
    vector<v_id> result;
    vector<v_id> degrees;
    result.reserve(g.size());
    degrees.reserve(g.size());

    for (v_id i = 0; i < g.size(); i++) { degrees.push_back(g.degreeOf(i)); }
    GraphLinearHeap heap(g.size(), g.size(), degrees);
    for (v_id i = 0; i < g.size(); i++) {
        auto smallestDeg = heap.popMin();
        result.push_back(smallestDeg.first);
    }
    return result;
}


std::ostream &operator<<(std::ostream &os, const Graph &graph) {
    return os << "Graph{size=" << graph.size() << "}";
}

// std::vector<v_id> degenOrdering(const Graph &g) {
//     uint32_t size = g.size();
//     ListLinearHeap heap{size, size};

//     vector<v_id> ids(g.size(), 0);
//     vector<v_id> degrees(g.size(), 0);

//     iota(ids.begin(), ids.end(), 0);
//     for (auto id : ids) { degrees[id] = g.degreeOf(id); }
// }

// -- Node
std::ostream &operator<<(std::ostream &os, const Vertex &vertex) {
    return os << "Vertex{" << vertex.getId() << "}";
}

} // namespace gm
