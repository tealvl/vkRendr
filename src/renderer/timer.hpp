#include <chrono>

class Timer {
public:
    Timer() {
        lastTimePoint = std::chrono::high_resolution_clock::now();
    }

    void update() {
        auto currentTimePoint = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTimePoint - lastTimePoint).count();
        lastTimePoint = currentTimePoint;
    }

    float getDeltaTime() const {
        return deltaTime;
    }

private:
    std::chrono::high_resolution_clock::time_point startTimePoint;
    std::chrono::high_resolution_clock::time_point lastTimePoint;
    float deltaTime = 0.0f;
};