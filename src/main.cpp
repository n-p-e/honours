#include <getopt.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include "graph/graph.hpp"
#include "graph/graphv2.hpp"
#include "graph/kdefective.hpp"
#include "graph/kplex.hpp"

using namespace std;

const char *USAGE = //
    "Usage:\n"
    "    --help, -h     print help\n"
    "    -p             select program to run\n"
    "    -g             path to input graph\n"
    "    -a             algorithm version\n";

option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"program", required_argument, NULL, 'p'},
    {"graph", required_argument, NULL, 'g'},
    {"algorithm", required_argument, NULL, 'a'},
    {0, 0, 0, 0}, // end of args
};

int main(int argc, char **argv) {
    int ch, k = 0;
    bool help = false;
    string program = "kplex", graphPath, algo = "v2";

    while ((ch = getopt_long(argc, argv, "g:a:p:k:h", longopts, NULL)) != -1) {
        // cout << std::format("{} {}\n", char(ch), optarg ? optarg : "");
        switch (ch) {
        case 'h':
            help = true;
            break;
        case 'p':
            program = optarg;
            break;
        case 'g':
            graphPath = optarg;
            break;
        case 'a':
            algo = optarg;
            break;
        case 'k':
            k = strtol(optarg, NULL, 10);
            break;
        default:
            help = true;
        }
    }

    if (help || graphPath.empty()) {
        cout << USAGE;
        return 0;
    }

    if (program == "kplex") {
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
        cout << "[solution] ";
        for (auto it = result.kPlex.begin(); it < result.kPlex.end(); it++) {
            if (it == result.kPlex.begin()) {
                cout << (*it);
            } else {
                cout << " " << (*it);
            }
        }
        cout << endl;
        if (!gm::validateKPlex(graph, result.kPlex, k)) {
            cout << "ERROR: !!!!!!Invalid kplex!!!!!!" << endl;
            exit(1);
        }
    } else if (program == "kdef") {
        gm::v2::GraphV2 graph = gm::v2::GraphV2::readFromFile(graphPath);
        cout << "[input graph] " << graph << endl;
        auto start = chrono::high_resolution_clock::now();

        gm::kDefResult result;
        result = gm::kDefDegenV2(graph, k);
        cout << "[kDef] found k-defective-clique of size " << result.size << endl;
        auto end = chrono::high_resolution_clock::now();
        cout << "[timer] " << chrono::duration_cast<chrono::microseconds>(end - start).count()
             << " microseconds" << endl;
        if (!gm::checkKDefV2(graph, result.kDefective, result.size)) {
            cout << "ERROR: !!!!!!Invalid k-defective-clique!!!!!!" << endl;
            exit(1);
        }
    }

    return 0;
}
