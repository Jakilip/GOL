#include "LsRandom.h"

#include <random>
#include <algorithm>

LsRandom::LsRandom() : LsRandom(std::random_device{}()) {}

LsRandom::LsRandom(unsigned seed) : randomEngine(seed), seed(seed) {}

int LsRandom::getRandomInt(int min, int max) {
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(randomEngine);
}

float LsRandom::getRandomFloat(float min, float max) {
    if (min > max) std::swap(min, max);
    if (min == max) return min;
    std::uniform_real_distribution<float> dist(min, max);
    return dist(randomEngine);
}

bool LsRandom::getRandomBool(float truthChance) {
    std::bernoulli_distribution dist(std::clamp(static_cast<double>(truthChance), 0.0, 1.0));
    return dist(randomEngine);
}