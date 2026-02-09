#ifndef CUDAHISTOGRAMS_TABLESTATS_HPP
#define CUDAHISTOGRAMS_TABLESTATS_HPP

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include "Timer.hpp"

class TimingTablePrinter {
public:
    static constexpr auto NAME_HEADER = "Test Name";
    static constexpr auto SIZE_HEADER = "Elements";
    static constexpr auto TIME_HEADER = "Time (ms)";

    static void print() {
        using namespace timing;

        std::lock_guard<std::mutex> lock(mutex());

        if (totals().empty()) {
            std::cout << "No timing data collected.\n";
            return;
        }



        size_t nameWidth = std::strlen(NAME_HEADER);
        size_t sizeWidth = std::strlen(SIZE_HEADER);
        size_t timeWidth = std::strlen(TIME_HEADER);

        for (const auto& [key, time] : totals()) {
            nameWidth = std::max(nameWidth, key.testName.size());
            sizeWidth = std::max(sizeWidth, std::to_string(key.dataSize).size());
            timeWidth = std::max(timeWidth, formatTime(time).size());
        }

        const auto separator = makeSeparator(nameWidth, sizeWidth, timeWidth);

        std::cout << separator << "\n";
        printHeader(nameWidth, sizeWidth, timeWidth);
        std::cout << separator << "\n";

        for (const auto& [key, time] : totals()) {
            std::cout << "| " << std::left  << std::setw(nameWidth) << key.testName
                      << " | " << std::right << std::setw(sizeWidth) << key.dataSize
                      << " | " << std::right << std::setw(timeWidth) << formatTime(time)
                      << " |\n";
        }

        std::cout << separator << "\n";
    }

private:
    static std::string formatTime(double time) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << time;
        return oss.str();
    }

    static std::string makeSeparator(const size_t nameW, const size_t sizeW, const size_t timeW) {
        return "+"
             + std::string(nameW + 2, '-')
             + "+" + std::string(sizeW + 2, '-')
             + "+" + std::string(timeW + 2, '-')
             + "+";
    }

    static void printHeader(const size_t nameW, const size_t sizeW, const size_t timeW) {
        std::cout
            << "| " << std::left  << std::setw(nameW) << NAME_HEADER
            << " | " << std::right << std::setw(sizeW) << SIZE_HEADER
            << " | " << std::right << std::setw(timeW) << TIME_HEADER
            << " |\n";
    }
};

#endif // CUDAHISTOGRAMS_TABLESTATS_HPP