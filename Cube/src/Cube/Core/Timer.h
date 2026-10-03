#pragma once

#include <chrono>

namespace Cube {

class Timer {
public:
    Timer();
    ~Timer() = default;

    double elapsed();
    void restart();
    double tick();
    
private:
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> lastTime;
};

}