#pragma once

#include "LsCore.h"

template<typename T>
bool pointsAreEqual(const LsVec2<T>& a, const LsVec2<T>& b) {
    return static_cast<double>(std::abs(a.x - b.x)) < LsEpsilon && static_cast<double>(std::abs(a.y - b.y)) < LsEpsilon;
}

float distance(const LsVec2F& a, const LsVec2F& b);
double distance(const LsVec2D& a, const LsVec2D& b);
float angleTo(const LsVec2F& observer, const LsVec2F& target);
double angleTo(const LsVec2D& observer, const LsVec2D& target);

