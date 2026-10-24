#include "GOL.hpp"

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

    //star of david manually contructed
    graphic.addLine({{50, 0}, {100, 86.603f}}, 0);
    graphic.addLine({{100, 86.603f}, {0, 86.603f}}, 0);
    graphic.addLine({{0, 86.603f}, {50, 0}}, 0);
    graphic.addLine({{0, 28.868f}, {100, 28.868f}}, 0);
    graphic.addLine({{100, 28.868f}, {50, 115.470f}}, 0);
    graphic.addLine({{50, 115.470f}, {0, 28.868f}}, 0);

    graphic.setColor(CYAN, 0);


    LsObject israel;

    israel.setSourceGraphic(graphic);
    israel.setPosition({2047.f, 2047.f});
    israel.setScale({15.f, 15.f});


    LsRenderer rendr;

    rendr.setDevice(0);
    if(!rendr.deviceIsValid()) {
        std::cout << "Device invalid!";
        return 1;
    }
    rendr.setFramerate(60);
    rendr.setMaxPPS(10000);


    LsTimer dtTimer;
    float degreesPerSecond = 200.f;
    float totalTime = 0.f;

    while(totalTime < 10.f){

        float dt = dtTimer.getTimeAsSeconds();
        totalTime += dt;
        dtTimer.restart();

        israel.setRotation(israel.getRotation() + (degreesPerSecond * dt));

        // play around during playback
        if(totalTime >= 5.f)
            israel.getSourceGraphic().setColor(PINK, 0);
        if(totalTime >= 6.f)
            israel.getSourceGraphic().setColor(YELLOW, 0);
        if(totalTime >= 7.f)
            israel.getSourceGraphic().setColor(BLUE, 0);
        if(totalTime >= 8.f)
            israel.getSourceGraphic().setColor(GREEN, 0);
        if(totalTime >= 9.f) {
            israel.getSourceGraphic().setColor(RED, 0);
            israel.setScale({15.f - (totalTime - 9.f) * 15.f, 15.f - (totalTime - 9.f) * 15.f});
        }

        rendr.draw(israel);

        rendr.pushFrame();

    }

    return 0;
};