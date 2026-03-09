// Grub.cpp
#include "Grub.h"
#include "Config.h"

#ifndef PI
    #define PI 3.14159265358979323846f
#endif

Grub::Grub() {
    // Default constructor
    position = {0, 0};
    velocity = {0, 0};
    speed = 1.0f;
    sight = 100.0f;
    metabolism = 0.05f;
    mutationChance = Config::DEFAULT_MUTATION_CHANCE;
    hunger = 1.0f;
    thirst = 1.0f;
    alive = true;
    reproCooldown = 0.0f;
    flashTimer = 0;
    
    hasTarget = false;
    targetTimer = 0.0f;
    
    InitializeColor();
}

Grub::Grub(Vector2 pos, float s, float sightR, float metab, float mutChance) {
    position = pos;
    velocity = {0, 0};
    speed = clampVal(s, Config::SPEED_MIN, Config::SPEED_MAX);
    sight = clampVal(sightR, Config::SIGHT_MIN, Config::SIGHT_MAX);
    metabolism = clampVal(metab, Config::METAB_MIN, Config::METAB_MAX);
    mutationChance = clampVal(mutChance, Config::MUTATION_CHANCE_MIN, Config::MUTATION_CHANCE_MAX);
    hunger = 1.0f;
    thirst = 1.0f;
    alive = true;
    reproCooldown = 0.0f;
    flashTimer = 0;
    
    hasTarget = false;
    targetTimer = 0.0f;
    
    InitializeColor();
    
    // Random initial velocity - using defined PI
    float ang = randFloat(0.0f, 2.0f * PI);
    velocity.x = cosf(ang) * speed;
    velocity.y = sinf(ang) * speed;
}

void Grub::InitializeColor() {
    // Change to lime color
    baseColor = LIME;
    drawColor = baseColor;
}

bool Grub::NeedsFoodOrWater() const {
    return hunger < 0.8f || thirst < 0.8f;
}

void Grub::MoveTowards(const Vector2 &target, float dt) {
    Vector2 dir = Vector2Subtract(target, position);
    float len = Vector2Length(dir);
    if (len > 0.001f) {
        dir = Vector2Scale(dir, 1.0f / len);
        velocity = Vector2Scale(dir, speed);
        position = Vector2Add(position, Vector2Scale(velocity, dt));
    }
}

void Grub::SetRandomTarget() {
    // Choose random angle (0 to 2π)
    float angle = randFloat(0.0f, 2.0f * PI);
    
    // Choose random distance within sight radius (full sight range)
    float distance = randFloat(0.0f, sight);
    
    // Calculate target point
    targetPosition.x = position.x + cosf(angle) * distance;
    targetPosition.y = position.y + sinf(angle) * distance;
    hasTarget = true;
    targetTimer = 0.0f;
}

void Grub::Wander(float dt) {
    // If no target or target reached, set new random target
    if (!hasTarget || targetTimer >= TARGET_CHANGE_INTERVAL) {
        SetRandomTarget();
    }
    
    // Move toward current target
    if (hasTarget) {
        MoveTowards(targetPosition, dt);
        targetTimer += dt;
        
        // Check if close to target
        float distToTarget = Vector2Distance(position, targetPosition);
        if (distToTarget < 5.0f) {  // Close enough
            SetRandomTarget();  // Get new target
        }
    }
}

void Grub::ClearTarget() {
    hasTarget = false;
    targetTimer = 0.0f;
}

void Grub::ApplyBounds(int envW, int envH) {
    const float bounceFactor = 0.8f;
    if (position.x < size) { 
        position.x = size; 
        velocity.x = fabsf(velocity.x) * bounceFactor; 
    }
    if (position.y < size) { 
        position.y = size; 
        velocity.y = fabsf(velocity.y) * bounceFactor; 
    }
    if (position.x > envW - size) { 
        position.x = envW - size; 
        velocity.x = -fabsf(velocity.x) * bounceFactor; 
    }
    if (position.y > envH - size) { 
        position.y = envH - size; 
        velocity.y = -fabsf(velocity.y) * bounceFactor; 
    }
}

void Grub::UpdateBars(float fixedStep) {
    // Current base drain
    float baseDrain = metabolism * fixedStep * 0.1f;
    
    // Move cost for higher speeds
    float moveCost = Vector2Length(velocity) * 0.001f * fixedStep; // Reduced from 0.005f
    
    // Speed cost
    float speedCost = speed * 0.00005f * fixedStep; // Reduced from 0.001f
    
    // Sight cost
    float sightCost = sight * Config::THIRST_COST_PER_SIGHT * fixedStep;
    
    // Apply drains with costs
    hunger -= (baseDrain + moveCost + speedCost);
    thirst -= (baseDrain + moveCost + sightCost);
    
    hunger = clampVal(hunger, 0.0f, 1.0f);
    thirst = clampVal(thirst, 0.0f, 1.0f);
    
    if (hunger <= 0.0f || thirst <= 0.0f) alive = false;
    if (reproCooldown > 0.0f) reproCooldown = std::max(0.0f, reproCooldown - fixedStep);
    
    // Handle color flash
    if (flashTimer > 0) {
        flashTimer--;
        if (flashTimer == 0) {
            ResetDrawColor();
        }
    }
}

bool Grub::CanReproduce() const {
    return alive && hunger >= 0.75f && thirst >= 0.75f && reproCooldown <= 0.0f;
}

void Grub::Feed(float foodValue) {
    hunger = clampVal(hunger + foodValue, 0.0f, 1.0f);
    drawColor = WHITE;
    flashTimer = 10;
}

void Grub::Drink(float waterValue) {
    thirst = clampVal(thirst + waterValue, 0.0f, 1.0f);
    drawColor = WHITE;
    flashTimer = 10;
}

void Grub::ResetDrawColor() { 
    drawColor = baseColor; 
}

void Grub::StartReproCooldown() { 
    reproCooldown = Config::REPRO_COOLDOWN; 
}

void Grub::Draw() const {
    if (!alive) {
        DrawCircleV(position, size, Fade(GRAY, 0.5f));
        return;
    }
    
    // Draw main grub body
    DrawCircleV(position, size, drawColor);
    
    // Draw two little eyes
    float eyeOffset = size * 0.4f;
    float eyeSize = size * 0.2f;
    
    // Left eye
    DrawCircle(position.x - eyeOffset, position.y - eyeOffset, eyeSize, BLACK);
    // Right eye
    DrawCircle(position.x + eyeOffset, position.y - eyeOffset, eyeSize, BLACK);
}