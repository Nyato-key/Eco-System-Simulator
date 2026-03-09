// Utils.h
#pragma once
#include <random>
#include <algorithm>
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Random number generator
extern std::mt19937 rng;

// Initialize RNG
void initRNG();
void initRNG(unsigned seed); // for reproducible results

// Utility functions
inline float randFloat(float a, float b) {
    std::uniform_real_distribution<float> d(a, b);
    return d(rng);
}

inline int randInt(int a, int b) {
    std::uniform_int_distribution<int> d(a, b);
    return d(rng);
}

// Random angle in radians
inline float randAngle() {
    return randFloat(0.0f, 2.0f * (float)M_PI);
}

template<typename T>
T clampVal(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}