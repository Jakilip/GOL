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

    // device params
    unsigned scanRate = 15000;
    unsigned framerate = 50;
    unsigned deviceID = UINT_MAX;

    // optimization params
    unsigned corneringLevel = 4;
    unsigned accelerationLevel = 2;
    unsigned blankingLevel = 2;
    bool dynamicFramerate = false;
    bool optimizePathing = false;
public:

    bool setDevice(unsigned deviceID);
    void setFramerate(unsigned framerate);
    void setScanRate(unsigned scanRate);
    void setOptimizationConfig(unsigned corneringLevel, unsigned accelerationLevel, unsigned blankingLevel, bool dynamicFramerate);

    void closeDAC();
    unsigned getDeviceID();
    unsigned getScanRate();
    unsigned getFramerate();

    void draw(LsObject& object);
    void pushFrame();
    void DEBUG_optimal_output() {
        buildOptimalOutput();
    }

    LsProjector();
    ~LsProjector();
};