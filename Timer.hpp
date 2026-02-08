//
// Created by Steven Roddan on 2/6/2026.
//

#ifndef CUDAHISTOGRAMS_TIMER_HPP
#define CUDAHISTOGRAMS_TIMER_HPP

#include <chrono>
#include <unordered_map>
#include <mutex>

namespace timing {

    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;

    inline std::unordered_map<std::string, double>& totals() {
        static std::unordered_map<std::string, double> map;
        return map;
    }

    inline std::unordered_map<std::string, time_point>& starts() {
        static std::unordered_map<std::string, time_point> map;
        return map;
    }

    inline std::mutex& mutex() {
        static std::mutex m;
        return m;
    }

}


#define TIMING_BEGIN(KEY)                           \
do {                                                \
std::lock_guard<std::mutex> lock(timing::mutex());  \
timing::starts()[KEY] = timing::clock::now();       \
} while (0)

#define TIMING_END(KEY)                             \
do {                                                \
auto end = timing::clock::now();                    \
std::lock_guard<std::mutex> lock(timing::mutex());  \
auto start = timing::starts().at(KEY);              \
double ms = std::chrono::duration<double, std::milli>(end - start).count(); \
timing::totals()[KEY] += ms;                        \
} while (0)

#endif //CUDAHISTOGRAMS_TIMER_HPP