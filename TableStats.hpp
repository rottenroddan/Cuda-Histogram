#ifndef CUDAHISTOGRAMS_TABLESTATS_HPP
#define CUDAHISTOGRAMS_TABLESTATS_HPP

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <map>

#include "Timer.hpp"

class TimingTablePrinter {
public:
    static constexpr auto NAME_HEADER   = "Test Name";
    static constexpr auto BIN_HEADER    = "Bin Size";
    static constexpr auto SIZE_HEADER   = "Elements";
    static constexpr auto TIME_HEADER   = "Time (ms)";
    static constexpr auto SPEED_HEADER  = "Speedup (x)";
    static constexpr auto THREAD_HEADER = "Threads";

    static void print(std::ostream& os = std::cout) {
        using namespace timing;

        std::lock_guard<std::mutex> lock(mutex());

        if (totals().empty()) {
            os << "No timing data collected.\n";
            return;
        }

        // -----------------------------
        // Build baseline lookup table
        // -----------------------------
        std::map<std::pair<size_t, size_t>, double> baselineTimes;

        for (const auto& [key, time] : totals()) {
            if (key.testName == "Baseline") {
                baselineTimes[{key.binSize, key.dataSize}] = time;
            }
        }

        size_t nameWidth   = std::strlen(NAME_HEADER);
        size_t threadWidth = std::strlen(THREAD_HEADER);
        size_t binWidth    = std::strlen(BIN_HEADER);
        size_t sizeWidth   = std::strlen(SIZE_HEADER);
        size_t timeWidth   = std::strlen(TIME_HEADER);
        size_t speedWidth  = std::strlen(SPEED_HEADER);

        std::map<timing::Key, double> computedSpeedups;

        for (const auto& [key, time] : totals()) {

            double speedup = 1.0;

            if (key.testName != "Baseline") {
                if (auto it = baselineTimes.find({key.binSize, key.dataSize});
                    it != baselineTimes.end() && time > 0.0) {
                    speedup = it->second / time;
                } else {
                    speedup = 0.0;
                }
            }

            computedSpeedups[key] = speedup;

            nameWidth   = std::max(nameWidth, key.testName.size());
            threadWidth = std::max(threadWidth, std::to_string(key.threadCount).size());
            binWidth    = std::max(binWidth,  std::to_string(key.binSize).size());
            sizeWidth   = std::max(sizeWidth, std::to_string(key.dataSize).size());
            timeWidth   = std::max(timeWidth, formatTime(time).size());
            speedWidth  = std::max(speedWidth, formatSpeedup(speedup).size());
        }

        const auto separator =
            makeSeparator(nameWidth, threadWidth, binWidth, sizeWidth, timeWidth, speedWidth);

        os << separator << "\n";
        printHeader(os, nameWidth, threadWidth, binWidth, sizeWidth, timeWidth, speedWidth);
        os << separator << "\n";

        for (const auto& [key, time] : totals()) {
            const double speedup = computedSpeedups[key];

            os << "| " << std::left  << std::setw(nameWidth)   << key.testName
               << " | " << std::right << std::setw(threadWidth) << key.threadCount
               << " | " << std::right << std::setw(binWidth)    << key.binSize
               << " | " << std::right << std::setw(sizeWidth)   << key.dataSize
               << " | " << std::right << std::setw(timeWidth)   << formatTime(time)
               << " | " << std::right << std::setw(speedWidth)  << formatSpeedup(speedup)
               << " |\n";
        }

        os << separator << "\n";
    }

private:

    static std::string formatTime(double time) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << time;
        return oss.str();
    }

    static std::string formatSpeedup(double speed) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << speed << "x";
        return oss.str();
    }

    static std::string makeSeparator(const size_t nameW,
                                     const size_t threadW,
                                     const size_t binW,
                                     const size_t sizeW,
                                     const size_t timeW,
                                     const size_t speedW) {
        return "+"
             + std::string(nameW + 2, '-')
             + "+" + std::string(threadW + 2, '-')
             + "+" + std::string(binW + 2, '-')
             + "+" + std::string(sizeW + 2, '-')
             + "+" + std::string(timeW + 2, '-')
             + "+" + std::string(speedW + 2, '-')
             + "+";
    }

    static void printHeader(std::ostream& os,
                            const size_t nameW,
                            const size_t threadW,
                            const size_t binW,
                            const size_t sizeW,
                            const size_t timeW,
                            const size_t speedW) {
        os
            << "| " << std::left  << std::setw(nameW)   << NAME_HEADER
            << " | " << std::right << std::setw(threadW) << THREAD_HEADER
            << " | " << std::right << std::setw(binW)    << BIN_HEADER
            << " | " << std::right << std::setw(sizeW)   << SIZE_HEADER
            << " | " << std::right << std::setw(timeW)   << TIME_HEADER
            << " | " << std::right << std::setw(speedW)  << SPEED_HEADER
            << " |\n";
    }
};

#endif