#pragma once

#include "../LsUtilities/LsCore.h"
#include "LsGraphic.h"

class LsObject {
private:
    friend class LsProjector;
    LsGraphic sourceGraphic{};
    LsGraphic graphic{};

    LsVec2F position = {0.f, 0.f};
    LsVec2F scale = {1.f, 1.f};
    float rotation = 0.f;

    LsVec2F sourceCenter;
    LsVec2F computeSourceCenter();
    void applyTransformation();
public:
    void setRotation(const float& newRotation);
    void setScale(LsVec2F newScale);
    void setPosition(LsVec2F newPosition);
    void setSourceGraphic(const LsGraphic& sourceGraphic);

    LsVec2F getPosition();
    LsVec2F getScale();
    float getRotation();

    LsGraphic getTransformedGraphic();
    LsGraphic& getSourceGraphic();

    LsObject() = default;
    LsObject(const LsGraphic& lines);
    LsObject(const LsVec2F& position, const LsVec2F& scale, const float& rotation, const LsGraphic& lines);
};