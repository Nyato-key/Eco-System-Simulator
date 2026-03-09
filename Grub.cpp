// Grub.cpp
#include "Grub.h"
#include "Config.h"

// Add PI definition if not available from raymath
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
    
    // NEW: Initialize target system
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
    
    // NEW: Initialize target system
    hasTarget = false;
    targetTimer = 0.0f;
    
    InitializeColor();
    
    // Random initial velocity - using defined PI
    float ang = randFloat(0.0f, 2.0f * PI);
    velocity.x = cosf(ang) * speed;
    velocity.y = sinf(ang) * speed;
}

void Grub::InitializeColor() {
    // Change to lime color instead of trait-based coloring
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
void Grub::ApplyBounds(int envW, int envH) {
    
    const float bounceFactor = 0.9f;
    bool hitBoundary = false;
    
    // Standard boundary check with bounce
    if (position.x < size) { 
        position.x = size; 
        velocity.x = fabsf(velocity.x) * bounceFactor;
        hitBoundary = true;
    }
    if (position.y < size) { 
        position.y = size; 
        velocity.y = fabsf(velocity.y) * bounceFactor;
        hitBoundary = true;
    }
    if (position.x > envW - size) { 
        position.x = envW - size; 
        velocity.x = -fabsf(velocity.x) * bounceFactor;
        hitBoundary = true;
    }
    if (position.y > envH - size) { 
        position.y = envH - size; 
        velocity.y = -fabsf(velocity.y) * bounceFactor;
        hitBoundary = true;
    }
    
    // If hit boundary, immediately get new target
    if (hitBoundary) {
        hasTarget = false;  // Force new target in next Wander call
        targetTimer = TARGET_CHANGE_INTERVAL;  // Force immediate target change
    }
}

// Update Wander to avoid edges:
void Grub::Wander(float dt, int envW, int envH) {
    // Add edge avoidance
    envW = 1200, envH = 800;  // Default, should match simulation
    
    // If very close to edge, set target away from edge
    float dangerZone = 10.0f;
    if (position.x < dangerZone || position.x > envW - dangerZone ||
        position.y < dangerZone || position.y > envH - dangerZone) {
        // Force new target away from edge
        hasTarget = false;
        targetTimer = TARGET_CHANGE_INTERVAL;
    }
    
    // Original wander logic
    if (!hasTarget || targetTimer >= TARGET_CHANGE_INTERVAL) {
        SetRandomTarget();
    }
    
    if (hasTarget) {
        MoveTowards(targetPosition, dt);
        targetTimer += dt;
        
        float distToTarget = Vector2Distance(position, targetPosition);
        if (distToTarget < 10.0f) {
            SetRandomTarget();
        }
    }
}

void Grub::ClearTarget() {
    hasTarget = false;
    targetTimer = 0.0f;
}
// In Grub.cpp UpdateBars method:
void Grub::UpdateBars(float fixedStep) {
    // BASE DRAIN = Metabolism × Time

    // Example: If metabolism = 0.1
    // baseDrain = 0.1 × 0.0167 × 0.1 = 0.000167 per frame
    // That's how much hunger/thirst is lost JUST FOR EXISTING
    float baseDrain = metabolism * fixedStep * 0.1f;

    // Adjust move cost for higher speeds - REDUCED cost
    float moveCost = Vector2Length(velocity) * 0.001f * fixedStep;
    
    // Speed cost - adjust for higher speeds
    float speedCost = speed * 0.00005f * fixedStep;
    
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