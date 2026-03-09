// Resource.cpp
#include "Resource.h"
#include "Utils.h"

void Resource::Respawn(int envW, int envH) {
    position.x = randFloat(30.0f, envW - 30.0f);
    position.y = randFloat(30.0f, envH - 30.0f);
    active = true;
}

void Resource::Draw() const {
    if (!active) return;
    
    if (isWater) {
        // Water drop with gradient effect
        DrawCircleV(position, Config::WATER_RADIUS, Fade(SKYBLUE, 0.9f));
        DrawCircleV(position, Config::WATER_RADIUS * 0.6f, Fade(BLUE, 0.7f));
        DrawCircleLines(position.x, position.y, Config::WATER_RADIUS, BLUE);
    } else {
        // Apple-like food with stem
        DrawCircleV(position, Config::FOOD_RADIUS, Fade(RED, 0.95f));
        DrawCircleLines(position.x, position.y, Config::FOOD_RADIUS, MAROON);
        
        // Add a small stem
        DrawRectangle(position.x - 1, position.y - Config::FOOD_RADIUS - 3, 2, 4, BROWN);
    }
}