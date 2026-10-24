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

    LsVec2U canvasSize = {4095, 4095};
    LsVec2U colorRange = {0, 255};

    int validDevicesAmount;
    unsigned maxPPS = 15000;
    unsigned framerate = 60;
    unsigned deviceID = UINT_MAX;
public:
    bool setDevice(int deviceID);
    void setFramerate(unsigned framerate);
    void setMaxPPS(unsigned maxPPS);

    void closeDAC();
    int getDeviceID();
    bool deviceIsValid();
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