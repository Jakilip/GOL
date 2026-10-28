#include <iostream>

#include "GOL.h"

using LsPadManager = LsControllerManager;
using LsPadButton = LsControllerButton;
using LsPadAxis = LsControllerAxis;

void controllerTest() {
    bool yes = true;
    bool no = true;
    LsPadManager mng;
    while (yes || no) {
        mng.updateInputs();

        if(mng.controllerIsActive(0)) {
            if(mng.isButtonPressed(0, LsPadButton::TOUCHPAD)) {
                yes = false;
            }
            std::cout << "Left Pedal controller 0: " << mng.getAxisValue(0, LsPadAxis::LEFT_PEDAL) << "\n";
        }
        else
            std::cerr << "Controller 0 not active\n";

        if(mng.controllerIsActive(1)) {
            if(mng.isButtonPressed(1, LsPadButton::TOUCHPAD)) {
                no = false;
            }
            std::cout << "Left Pedal controller 1: " << mng.getAxisValue(1, LsPadAxis::LEFT_PEDAL) << "\n";
        }
        else
            std::cerr << "Controller 1 not active\n";

        Sleep(30);
    }
}

int main() {

    LsGraphic graphic;

    graphic.addLine({{25000, 0}, {50000, 43301}}, 0);
    graphic.addLine({{50000, 43301}, {0, 43301}}, 0);
    graphic.addLine({{0, 43301}, {25000, 0}}, 0);
    graphic.addLine({{0, 14434}, {50000, 14434}}, 0);
    graphic.addLine({{50000, 14434}, {25000, 57735}}, 0);
    graphic.addLine({{25000, 57735}, {0, 14434}}, 0);

    graphic.setColor(LsCyan, 0);

    graphic.logContents();
    graphic.saveToFile("format5-save.ild");

    graphic.loadFromFile("format5-save.ild");
    graphic.logContents();

    return 0;
};