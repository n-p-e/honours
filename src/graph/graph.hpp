#ifndef GM_GRAPH_HPP
#define GM_GRAPH_HPP

#include "graph/types.hpp"
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

namespace gm {

class Graph;
class Vertex;


Graph readGraph(const std::string &path);

class Graph {
private:
    v_int size_;
    std::vector<Vertex> v;
    std::vector<std::vector<v_id>> adjs;

public:
    explicit Graph(int64_t size);
    Graph(const Graph &) = default; // copy
    Graph(Graph &&) = default;      // move

    uint64_t size() const;
    const std::vector<v_id> &neighbours(v_id vertex) const;
    int64_t degreeOf(v_id vertex) const;

    void addEdge(v_id v1, v_id v2);

    /// Create a subgraph from a list of vertices
    Graph subgraph(const std::vector<v_id> &vertices) const;

    friend std::ostream &operator<<(std::ostream &os, const Graph &graph);
};

/// Calculate a degeneracy ordering of a graph
std::vector<v_id> degenOrdering(const Graph &g);

class Vertex {
private:
    Graph *graph;
    v_id id;

public:
    inline Vertex(Graph *graph, v_id id) : graph(graph), id(id) {}
    inline v_id getId() const {
        return this->id;
    }
    friend std::ostream &operator<<(std::ostream &os, const Vertex &node);
};


}; // namespace gm

#endif // GM_GRAPH_HPP
