#include "LsObject.h"

#include <algorithm>
#include <cmath>

//save and load are implemented in ILDAFormat.cpp

LsVec2F LsObject::getPosition() { return position; }
float LsObject::getRotation() { return rotation; }
LsVec2F LsObject::getScale() { return scale; }
LsGraphic LsObject::getTransformedGraphic() { applyTransformation(); return graphic; }
LsGraphic& LsObject::getSourceGraphic() { return sourceGraphic; }

void LsObject::setPosition(LsVec2F newPosition) { this->position = newPosition; }
void LsObject::setScale(LsVec2F newScale) { this->scale = newScale; }
void LsObject::setRotation(const float& newRotation) { this->rotation = newRotation; }
void LsObject::setSourceGraphic(const LsGraphic& sourceGraphic) { this->sourceGraphic = sourceGraphic; this->graphic = sourceGraphic; this->sourceCenter = computeSourceCenter(); }

LsVec2F LsObject::computeSourceCenter() {
    float left = std::numeric_limits<float>::infinity();
    float right = -std::numeric_limits<float>::infinity();
    float top = -std::numeric_limits<float>::infinity();
    float bottom = std::numeric_limits<float>::infinity();

    for (auto& line : this->sourceGraphic.getLines()) {
        left = std::min({left, line.start.x, line.end.x});
        right = std::max({right, line.start.x, line.end.x});
        bottom = std::min({bottom, line.start.y, line.end.y});
        top = std::max({top, line.start.y, line.end.y});
    }

    LsVec2F center = { left + (right - left) / 2.f, bottom + (top - bottom) / 2.f };

    return center;
}

LsObject::LsObject(const LsGraphic& lines) {
    this->sourceGraphic = lines;
    this->graphic = lines;
    sourceCenter = this->computeSourceCenter();
}

LsObject::LsObject(const LsVec2F& position, const LsVec2F& scale, const float& rotation, const LsGraphic& lines) {
    this->position = position;
    this->scale = scale;
    this->rotation = rotation;
    this->sourceGraphic = lines;
    this->graphic = lines;
    sourceCenter = this->computeSourceCenter();
    this->applyTransformation();
}

void LsObject::applyTransformation() {
    this->graphic = this->sourceGraphic;

    auto& lines = graphic.lines;
    if (lines.empty())
        return;

    //rotation based on degrees
    const float angle = rotation * (std::numbers::pi_v<float> / 180.0f);
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);

    auto rotatePoint = [&](LsVec2F& p) {
        const float dx = p.x - sourceCenter.x;
        const float dy = p.y - sourceCenter.y;
        p.x = sourceCenter.x + dx * cosA - dy * sinA;
        p.y = sourceCenter.y + dx * sinA + dy * cosA;
    };

    for (auto& line : lines) {
        rotatePoint(line.start);
        rotatePoint(line.end);
    }

    //scale
    for(auto& line : lines) {
        line.start *= this->scale;
        line.end *= this->scale;
    }

    //position
    for(auto& line : lines) {
        line.start += this->position;
        line.end += this->position;
    }

    graphic.figureOutSize();
}
