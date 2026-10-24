#pragma once

#include <chrono>

class LsTimer {
private:
    std::chrono::time_point<std::chrono::steady_clock> startPoint;
public:
    void restart();
    float getTimeAsSeconds() const;
    float getTimeAsMilliseconds() const;
    float getTimeAsMicroseconds() const;

    LsTimer();
};
