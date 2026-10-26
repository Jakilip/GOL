#include "LsObject.h"

#include "../LsUtilities/LsCollisions.h"

#include <algorithm>
#include <cmath>

//save and load are implemented in ILDAFormat.cpp

LsVec2F LsObject::getPosition() { return position; }

float LsObject::getRotation() { return rotation; }

LsVec2F LsObject::getScale() { return scale; }

LsVec2F LsObject::getOrigin() { return originPointNormalized; }

LsGraphic LsObject::getTransformedGraphic() { applyTransformation(); return transformGraphic; }

LsGraphic& LsObject::getSourceGraphic() { return sourceGraphic; }

void LsObject::setPosition(LsVec2F newPosition) { this->position = newPosition; }

void LsObject::setScale(LsVec2F newScale) { this->scale = newScale; }

void LsObject::setRotation(float newRotation) { this->rotation = newRotation; }

void LsObject::setOrigin(LsVec2F newOrigin) { this->originPointNormalized = newOrigin; }

void LsObject::setSourceGraphic(const LsGraphic& sourceGraphic) { this->sourceGraphic = sourceGraphic; }

bool LsObject::collidesWith(LsObject &other) {
    this->applyTransformation();
    const auto& otherLines = other.getTransformedGraphic().lines;

    for(auto& thisLine : this->transformGraphic.lines) {
        for(auto& otherLine : otherLines) {
            if(linesCollide(thisLine, otherLine))
                return true;
        }
    }
    return false;
}

LsObject::LsObject(const LsGraphic& sourceGraphic) {
    this->sourceGraphic = sourceGraphic;
    this->transformGraphic = sourceGraphic;
}

void LsObject::applyTransformation() {
    this->transformGraphic = this->sourceGraphic;

    auto& lines = transformGraphic.lines;
    if(lines.empty())
        return;

    const LsVec2F origin = this->sourceGraphic.getSize() * this->originPointNormalized;

    const float angle = rotation * (std::numbers::pi_v<float> / 180.0f);
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);

    auto transformPoint = [&](LsVec2F& p) {
        // make relative to origin
        float dx = p.x - origin.x;
        float dy = p.y - origin.y;

        // scale (around origin)
        dx *= scale.x;   // use `scale` directly if it's a float
        dy *= scale.y;

        // rotate (around origin)
        const float rx = dx * cosA - dy * sinA;
        const float ry = dx * sinA + dy * cosA;

        // origin lands on position
        p.x = position.x + rx;
        p.y = position.y + ry;
    };

    for(auto& line : lines) {
        transformPoint(line.start);
        transformPoint(line.end);
    }
}
