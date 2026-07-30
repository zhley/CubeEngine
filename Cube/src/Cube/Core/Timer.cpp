#include "Timer.h"

namespace Cube {

	Timer::Timer() { startTime = std::chrono::steady_clock::now(); }

    float Timer::stop() {
        auto endTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> duration = endTime - startTime;
        return duration.count();
    }

    void Timer::restart() { startTime = std::chrono::steady_clock::now(); }

    float Timer::getDuration() {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> duration = currentTime - lastTime;
        lastTime = std::chrono::steady_clock::now();
        return duration.count();
	}

}  // namespace Cube