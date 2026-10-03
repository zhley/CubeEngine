#include "Timer.h"

namespace Cube {

Timer::Timer() : startTime(std::chrono::steady_clock::now()), lastTime(startTime) {}

double Timer::elapsed() {
    auto endTime = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(endTime - startTime).count();
}

void Timer::restart() { 
    startTime = std::chrono::steady_clock::now(); 
    lastTime = startTime;
}

double Timer::tick() {
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = currentTime - lastTime;
    lastTime = std::chrono::steady_clock::now();
    return duration.count();
}

}  // namespace Cube