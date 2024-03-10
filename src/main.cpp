#include "graph/graph.hpp"
#include "graph/kplex.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char **argv) {
    po::options_description desc("Program options");
    // clang-format off
    desc.add_options()
        ("help", "help message")
        ("algorithm,a", po::value<string>()->default_value("v2"), "select algorithm [v1 / v2(default)]")
        ("graph,g", po::value<string>(), "path for graph file")
        ("k,k", po::value<int>())
        ;
    // clang-format on

    po::variables_map args;
    po::store(po::parse_command_line(argc, argv, desc), args);

    string graphPath = args["graph"].as<string>();
    string algo = args["algorithm"].as<string>();
    int k = args["k"].as<int>();
    gm::Graph graph = gm::readGraph(graphPath);
    cout << "[input graph] " << graph << endl;
    gm::KPlexDegenResult result;
    auto start = chrono::high_resolution_clock::now();
    if (algo == "v1") {
        result = gm::kPlexDegen(graph, k);
        cout << "[kPlexDegen] Found k-plex of size " << result.kPlex.size() << "\n"
             << "    upper bound: " << result.ub << endl;
    } else if (algo == "v2") {
        result = gm::kPlexV2(graph, k, false);
        cout << "[kPlexV2] Found k-plex of size " << result.kPlex.size() << "\n"
             << "    upper bound: " << result.ub << endl;
    } else {
        result = gm::kPlexV2(graph, k, true);
        cout << "[kPlexTwoHop] Found k-plex of size " << result.kPlex.size() << "\n"
             << "    upper bound: " << result.ub << endl;
    }
    auto end = chrono::high_resolution_clock::now();
    cout << "[timer] " << chrono::duration_cast<chrono::microseconds>(end - start).count()
         << " microseconds" << endl;


    return 0;
}
