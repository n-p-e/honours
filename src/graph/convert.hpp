#ifndef GM_CONVERT_HPP
#define GM_CONVERT_HPP

#include <iostream>
#include <fstream>
#include <string>

#include "graph/graphv2.hpp"


namespace gm {

inline void convertGraph(std::string inputPath, std::string outputPath) {
    auto graph = v2::Graph::readFromFile(inputPath);
    std::ofstream out{outputPath};
    out << graph.size() << " " << (graph.eSize() / 2) << "\n";

    for (v_int i = 0; i < graph.size(); i++) {
        out << i;
        for (v_int v : graph.iterNeighbours(i)) { out << " " << v; }
        out << "\r\n";
    }
}

} // namespace gm

#endif
