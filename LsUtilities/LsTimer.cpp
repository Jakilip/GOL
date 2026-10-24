#include "LsTimer.h"

#include <chrono>

void LsTimer::restart() { this->startPoint = std::chrono::steady_clock::now(); }
LsTimer::LsTimer() { this->restart(); }
float LsTimer::getTimeAsMicroseconds() const { return std::chrono::duration<float, std::micro>(std::chrono::steady_clock::now() - startPoint).count(); }
float LsTimer::getTimeAsMilliseconds() const { return std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - startPoint).count(); }
float LsTimer::getTimeAsSeconds() const { return std::chrono::duration<float>(std::chrono::steady_clock::now() - startPoint).count(); }