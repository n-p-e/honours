#include <cstddef>
#include <getopt.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include "graph/graphv2.hpp"
#include "graph/kdefective.hpp"
#include "graph/kplex.hpp"
#include "graph/pseudoclique.hpp"
#include "graph/quasiclique.hpp"
#include "graph/types.hpp"
#include "util.hpp"

using namespace std;

static const char *USAGE = //
    "Usage:\n"
    "    --help, -h     print help\n"
    "    -p             select program to run\n"
    "    -g             path to input graph\n"
    "    -a             algorithm version\n";

constexpr int LONGOPT_ALPHA = 10001;
static option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"program", required_argument, NULL, 'p'},
    {"graph", required_argument, NULL, 'g'},
    {"algorithm", required_argument, NULL, 'a'},
    {"alpha", required_argument, NULL, LONGOPT_ALPHA},
    {0, 0, 0, 0}, // end of args
};

int main(int argc, char **argv) {
    int ch, k = 0;
    bool help = false;
    string program = "kplex", graphPath, algo = "v2";
    double alpha = 0.; // alpha for quasi-clique

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
        case LONGOPT_ALPHA:
            alpha = strtod(optarg, NULL);
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
        gm::v2::Graph graph = gm::v2::Graph::readFromFile(graphPath);
        cout << "[input graph] " << graph << endl;
        gm::KPlexDegenResult result;
        auto start = chrono::high_resolution_clock::now();
        if (algo == "v1") {
            result = gm::kPlexDegen(graph, k);
            cout << "[kPlexDegen] Result size = " << result.kPlex.size() << "\n"
                 << "    upper bound: " << result.ub << endl;
        } else if (algo == "v2") {
            result = gm::kPlexV2(graph, k, false);
            cout << "[kPlexV2] Result size = " << result.kPlex.size() << "\n"
                 << "    upper bound: " << result.ub << endl;
        } else {
            result = gm::kPlexV2(graph, k, true);
            cout << "[kPlexTwoHop] Result size = " << result.kPlex.size() << "\n"
                 << "    upper bound: " << result.ub << endl;
        }
        auto end = chrono::high_resolution_clock::now();
        cout << "[timer] " << chrono::duration_cast<chrono::microseconds>(end - start).count()
             << " microseconds" << endl;
        cout << "[solution] ";
        // comment when experimenting
        // for (auto it = result.kPlex.begin(); it < result.kPlex.end(); it++) {
        //     if (it == result.kPlex.begin()) {
        //         cout << (*it);
        //     } else {
        //         cout << " " << (*it);
        //     }
        // }
        cout << endl;
        if (!gm::validateKPlex(graph, result.kPlex, k)) {
            cout << "ERROR: !!!!!!Invalid kplex!!!!!!" << endl;
            exit(1);
        }
    } else if (program == "kdef") {
        gm::v2::Graph graph = gm::v2::Graph::readFromFile(graphPath);
        cout << "[input graph] " << graph << endl;
        if (algo == "twohop") { cout << "[kDef] using 2-hop neighbours\n"; }
        if (algo == "naive") { cout << "[kDef] using naive algo\n"; }
        auto start = chrono::high_resolution_clock::now();
        gm::kDefResult result;
        if (algo == "naive") {
            result = gm::kDefNaiveV2(graph, k);
        } else {
            result = gm::kDefDegenV2(graph, k, algo == "twohop");
        }
        cout << "[kDef] Result size = " << result.size << endl;
        auto end = chrono::high_resolution_clock::now();
        cout << "[timer] " << chrono::duration_cast<chrono::microseconds>(end - start).count()
             << " microseconds" << endl;
        if (!gm::checkKDefV2(graph, result.kDefective, k)) {
            cout << "ERROR: !!!!!!Invalid k-defective-clique!!!!!!" << endl;
            exit(1);
        }
    } else if (program == "quasi") {
        if (!(0 < alpha && alpha < 1)) {
            cout << "ERROR: provide --alpha as a number between 0 and 1" << endl;
            exit(1);
        }
        gm::v2::Graph graph = gm::v2::Graph::readFromFile(graphPath);
        cout << "[input graph] " << graph << endl;
        cout << format("[quasiClique] alpha={}\n", alpha);
        gm::SubgraphResult result;
        if (algo == "naive") {
            result = gm::printTimer([&]() { return gm::quasiCliqueNaive(graph, alpha); });
        } else {
            result = gm::printTimer([&]() { return gm::quasiClique(graph, alpha, algo == "twohop"); });
        }
        cout << format("[quasiClique] Result size = {}\n", result.size);
        if (!gm::validateQuasiClique(graph, result.subgraph, alpha)) {
            cout << "ERROR: !!!!!!Invalid quasiclique!!!!!!" << endl;
            exit(1);
        }
    } else if (program == "pseudo") {
        if (!(0 < alpha && alpha < 1)) {
            cout << "ERROR: provide --alpha as a number between 0 and 1" << endl;
            exit(1);
        }
        gm::v2::Graph graph = gm::v2::Graph::readFromFile(graphPath);
        cout << "[input graph] " << graph << endl;
        cout << format("[pseudoClique] alpha={}\n", alpha);
        gm::SubgraphResult result;
        if (algo == "naive") {
            result = gm::printTimer([&]() { return gm::pseudoCliqueNaive(graph, alpha); });
        } else {
            result = gm::printTimer([&]() { return gm::pseudoClique(graph, alpha, algo == "twohop"); });
        }
        cout << format("[pseudoClique] Result size = {}\n", result.size);
        if (!gm::validatePseudoClique(graph, result.subgraph, alpha)) {
            cout << "ERROR: !!!!!!Invalid pseudoclique!!!!!!" << endl;
            exit(1);
        }
    }

    return 0;
}
