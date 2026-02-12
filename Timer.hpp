#ifndef CUDAHISTOGRAMS_TIMER_HPP
#define CUDAHISTOGRAMS_TIMER_HPP

#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

namespace timing {

    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;

    struct Key {
        std::string testName;
        std::size_t binSize;
        std::size_t dataSize;

        bool operator<(const Key& other) const {
            return std::tie(testName, binSize, dataSize) <
                   std::tie(other.testName, other.binSize, other.dataSize);
        }
    };

    inline std::map<Key, double>& totals() {
        static std::map<Key, double> map;
        return map;
    }

    inline std::map<Key, time_point>& starts() {
        static std::map<Key, time_point> map;
        return map;
    }

    inline std::mutex& mutex() {
        static std::mutex m;
        return m;
    }
}

#define TIMING_BEGIN(NAME, BIN, SIZE)               \
do {                                                \
std::lock_guard<std::mutex> lock(timing::mutex()); \
timing::Key key{ NAME, BIN, SIZE };             \
timing::starts()[key] = timing::clock::now();   \
} while (0)

#define TIMING_END(NAME, BIN, SIZE)                 \
do {                                                \
auto end = timing::clock::now();                \
std::lock_guard<std::mutex> lock(timing::mutex()); \
timing::Key key{ NAME, BIN, SIZE };             \
auto start = timing::starts().at(key);          \
double ms = std::chrono::duration<double, std::milli>(end - start).count(); \
timing::totals()[key] += ms;                    \
} while (0)

#endif