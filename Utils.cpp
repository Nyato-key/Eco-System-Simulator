// Utils.cpp
#include "Utils.h"
#include <chrono>

// Initialize the random number generator
std::mt19937 rng;

void initRNG() {
    unsigned seed = (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(seed);
}

// For reproducible results with a fixed seed
void initRNG(unsigned seed) {
    rng.seed(seed);
}