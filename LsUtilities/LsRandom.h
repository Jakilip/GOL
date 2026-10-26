#pragma once
#include <random>

class LsRandom {
private:
    std::mt19937 randomEngine;
    unsigned seed;
public:

    int getRandomInt(int min, int max);
    float getRandomFloat(float min, float max);
    bool getRandomBool(float truthChance);

    LsRandom();
    LsRandom(unsigned seed);
};
