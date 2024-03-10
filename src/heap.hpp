#ifndef GM_HEAP_CPP
#define GM_HEAP_CPP

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

#include "graph/types.hpp"
#include "util.hpp"

namespace gm {

template<class Key, class Value>
class LinearHeap {
private:
    size_t size_;
    Value maxVal_;
    Value min_;
    // head nodes for each bucket, by value
    std::vector<Key> heads;

    // prev, next, values, by key
    std::vector<Key> prev;
    std::vector<Key> next;
    std::vector<Value> values;
    std::vector<bool> popped;

public:
    LinearHeap(size_t size, Value maxVal, const std::vector<Value> &initialValues)
        : size_(size), maxVal_(maxVal), min_(maxVal), heads(maxVal, -1), prev(size, -1),
          next(size, -1), values(initialValues), popped(size, false) {
        for (int i = 0; i < size; i++) {
            next[i] = heads[values[i]];
            if (heads[values[i]] != -1) { prev[heads[values[i]]] = i; }
            heads[values[i]] = i;
            if (values[i] < this->min_) { this->min_ = values[i]; }
        }
    }
    ~LinearHeap() {}

    Value size() const {
        return size_;
    }
    Value getById(Key key) const {
        GM_ASSERT(size() > 0, "Size should be >0");
        return values[key];
    }
    std::pair<Key, Value> getMin() const {
        GM_ASSERT(size() > 0, "Size should be >0");

        return std::make_pair(heads[min_], min_);
    }
    std::pair<Key, Value> popMin() {
        GM_ASSERT(size() > 0, "Size should be >0");

        auto answer = std::make_pair(heads[min_], min_);
        popped[answer.first] = true;
        size_ -= 1;
        heads[min_] = next[heads[min_]];
        if (heads[min_] != -1) { prev[heads[min_]] = -1; }
        while (min_ < maxVal_ && heads[min_] == -1) { min_ += 1; }
        return answer;
    }
    bool decrement(Key key, Value amount) {
        if (popped[key]) { return false; }
        // remove node
        if (next[key] != -1) { prev[next[key]] = prev[key]; }
        if (prev[key] != -1) { next[prev[key]] = next[key]; }
        if (key == heads[values[key]]) { heads[values[key]] = next[key]; }
        // add it
        Value newValue = values[key] - amount;
        values[key] = newValue;
        next[key] = heads[newValue];
        prev[key] = -1;
        if (heads[newValue] != -1) { prev[heads[newValue]] = key; }
        heads[newValue] = key;
        if (newValue < min_) { min_ = newValue; }
        return true;
    }
};

using GraphLinearHeap = LinearHeap<v_id, v_id>;

} // namespace gm

#endif // GM_HEAP_CPP
