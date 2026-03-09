// Grub.h - CORRECTED VERSION
#pragma once
#include "raylib.h"
#include "Config.h"
#include "Utils.h"
#include <raymath.h>

class Grub {
public:
    Vector2 position;
    Vector2 velocity;
    float size = 20.0f;

    // Internal state
    float hunger = 1.0f;
    float thirst = 1.0f;
    float speed;
    float sight;
    float metabolism;
    float mutationChance;
    
    Color baseColor;
    Color drawColor;
    int flashTimer = 0;
    bool alive = true;
    float reproCooldown = 0.0f;
    
    // NEW: Target system for movement
    Vector2 targetPosition;
    bool hasTarget = false;
    float targetTimer = 0.0f;
    static constexpr float TARGET_CHANGE_INTERVAL = 2.0f;

    // Constructors
    Grub();
    Grub(Vector2 pos, float s, float sightR, float metab, float mutChance);

    // Core methods
    bool NeedsFoodOrWater() const;
    void MoveTowards(const Vector2 &target, float dt);
    void Wander(float dt, int envW, int envH);
    void ApplyBounds(int envW, int envH);
    void UpdateBars(float fixedStep);
    bool CanReproduce() const;
    void Feed(float foodValue);
    void Drink(float waterValue);
    void ResetDrawColor();
    void StartReproCooldown();
    void Draw() const;
    
    // NEW: Target management methods
    void ClearTarget();
    void SetRandomTarget();
    bool HasTarget() const { return hasTarget; }
    Vector2 GetTarget() const { return targetPosition; }

    // GETTER METHODS - THESE MUST BE UNCOMMENTED/ACTUAL CODE
    float GetHunger() const { return hunger; }
    float GetThirst() const { return thirst; }
    float GetSpeed() const { return speed; }
    float GetSight() const { return sight; }
    float GetMetabolism() const { return metabolism; }
    float GetMutationChance() const { return mutationChance; }
    bool IsReproductive() const { return CanReproduce(); }
    float GetReproCooldown() const { return reproCooldown; }
    
private:
    void InitializeColor();
};