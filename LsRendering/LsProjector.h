#pragma once

#include "LsGraphic.h"
#include "LsObject.h"

#include "../DACs/HeliosDAC/HeliosDac.h"

class LsProjector{
private:
    HeliosDac dac;
    std::vector<LsGraphic> graphics;

    HeliosPoint makeHeliosPoint(LsVec2F pos, LsColor color);
    std::vector<HeliosPoint> buildOptimalOutput();

    struct LaserPoint {
        LsVec2F currentPos;
        float angleMoving; // degrees
    };
    LaserPoint laserPoint = {{0, 0}, 0};

    int validDevicesAmount;
    unsigned maxPPS = 5000;
    unsigned framerate = 50;
    unsigned deviceID = UINT_MAX;
public:
    bool setDevice(unsigned deviceID);
    void setFramerate(unsigned framerate);
    void setMaxPPS(unsigned maxPPS);

    void closeDAC();
    unsigned getDeviceID();
    unsigned getMaxPPS();
    unsigned getFramerate();

    void draw(LsObject& object);
    void pushFrame();
    void DEBUG_optimal_output() {
        buildOptimalOutput();
    }

    LsProjector();
    ~LsProjector();
};