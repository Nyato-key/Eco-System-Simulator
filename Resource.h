// Resource.h
#pragma once
#include "raylib.h"
#include "Config.h"

struct Resource {
    Vector2 position;
    float value;
    bool active;
    bool isWater;
    
    void Respawn(int envW, int envH);
    void Draw() const;
};