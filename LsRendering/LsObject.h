#pragma once

#include "../LsUtilities/LsCore.h"
#include "LsGraphic.h"

class LsObject {
private:
    friend class LsProjector;
    LsGraphic sourceGraphic{};
    LsGraphic transformGraphic{};

    LsVec2F position = {0.f, 0.f};
    LsVec2F scale = {1.f, 1.f};
    float rotation = 0.f;

    LsVec2F originPointNormalized = {0.f, 0.f};
    void applyTransformation();
public:
    void setRotation(float newRotation);
    void setScale(LsVec2F newScale);
    void setPosition(LsVec2F newPosition);
    void setOrigin(LsVec2F newOrigin);
    void setSourceGraphic(const LsGraphic& sourceGraphic);

    LsVec2F getPosition();
    LsVec2F getScale();
    float getRotation();
    LsVec2F getOrigin();

    LsGraphic getTransformedGraphic();
    LsGraphic& getSourceGraphic();

    bool collidesWith(LsObject& other);

    LsObject() = default;
    LsObject(const LsGraphic& sourceGraphic);
};