#include "LsProjector.h"

#include <iostream>

void LsProjector::draw(LsObject& object) {
    graphics.emplace_back(object.getTransformedGraphic());
}

//LsRenderer::buildOptimalOutput() implemented in isolated implementation because of complexicity

void LsProjector::pushFrame() {
    if(deviceID == UINT_MAX) {
        std::cerr << "Attempted to push frame with no valid device set\n";
        return;
    }

    //sophisticated bullshit to build the optimal points configuration
    std::vector<HeliosPoint> output = this->buildOptimalOutput();

    auto start = std::chrono::steady_clock::now();
    while(dac.GetStatus(deviceID) != 1) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        if (std::chrono::steady_clock::now() - start >= std::chrono::seconds(1)) {
            std::cerr << "LsRenderer::pushFrame no response from set device (" + std::to_string(deviceID) + ") within 1 second of waiting to send output\n";
            return;
        }
    }

    unsigned PPS = this->framerate * static_cast<unsigned>(output.size());
    if(PPS > maxPPS)
        PPS = maxPPS;

    dac.WriteFrame(deviceID, PPS, 0, output.data(), output.size());

    graphics.clear();
}

bool LsProjector::deviceIsValid() { return deviceID != UINT_MAX; }
int LsProjector::getDeviceID() { return this->deviceID; }
unsigned LsProjector::getFramerate() { return this->framerate; }
unsigned LsProjector::getMaxPPS() { return this->maxPPS; }
void LsProjector::setFramerate(unsigned framerate) { this->framerate = framerate; }
void LsProjector::setMaxPPS(unsigned maxPPS) { this->maxPPS = maxPPS; if (maxPPS > 30000)
    std::cout << "Warning: set maxPPS to above 30k, it's very rare for lasers to go this high, and requesting pointrates above the laser's limit can damage the hardware"; }

bool LsProjector::setDevice(int deviceID) {
    if(validDevicesAmount >= deviceID+1)
        this->deviceID = deviceID;
    else
        this->deviceID = UINT_MAX;
    return this->deviceIsValid();
}

LsProjector::LsProjector() {
    dac.CloseDevices();
    validDevicesAmount = dac.OpenDevices();
}

LsProjector::~LsProjector() {
    dac.CloseDevices();
    std::cout << "Cleaning up helios\n";
}

void LsProjector::closeDAC() {
    dac.CloseDevices();
    std::cout << "Cleaning up helios\n";
}