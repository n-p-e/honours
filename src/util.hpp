#ifndef GM_UTIL_HPP
#define GM_UTIL_HPP

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#define GM_ASSERT(val, message) ((val) ? 0 : (std::cout << (message) << std::endl, std::exit(1), 0))

namespace gm {

template<class T>
void printVector(const std::vector<T> &vec) {
    std::cout << "[";
    bool first = true;
    for (auto v : vec) {
        if (!first) { std::cout << ","; }
        first = false;
        std::cout << v;
    }
    std::cout << "]" << std::endl;
}

} // namespace gm

#endif // GM_UTIL_HPP
