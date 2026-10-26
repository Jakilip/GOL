#include "LsCollisions.h"

#include "LsCore.h"

#include <cmath>

bool linesCollide(LsLine a, LsLine b) {

    auto orient = [](const auto& p, const auto& q, const auto& r) {
        return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
    };

    auto onSegment = [](const auto& p, const auto& q, const auto& r) {
        return std::min(p.x, q.x) <= r.x && r.x <= std::max(p.x, q.x) &&
               std::min(p.y, q.y) <= r.y && r.y <= std::max(p.y, q.y);
    };

    auto straddles = [](float o1, float o2) {
        return (o1 > 0 && o2 < 0) || (o1 < 0 && o2 > 0);
    };

    auto isZero = [](float v) { return std::abs(v) < LsEpsilon; };

    float o1 = orient(a.start, a.end, b.start);
    float o2 = orient(a.start, a.end, b.end);
    float o3 = orient(b.start, b.end, a.start);
    float o4 = orient(b.start, b.end, a.end);

    if (straddles(o1, o2) && straddles(o3, o4))
        return true;

    return (isZero(o1) && onSegment(a.start, a.end, b.start)) ||
           (isZero(o2) && onSegment(a.start, a.end, b.end))   ||
           (isZero(o3) && onSegment(b.start, b.end, a.start)) ||
           (isZero(o4) && onSegment(b.start, b.end, a.end));
}