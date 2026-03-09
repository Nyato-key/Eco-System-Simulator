// ========================= Simulation.h =========================
#pragma once

#include "raylib.h"
#include <vector>
#include <string>

#include "Grub.h"
#include "Resource.h"
#include "UniformGrid.h"
#include "Config.h"

// ------------------------------------------------------------
// Resource claim structure
// Ensures only one grub targets a resource at a time
// ------------------------------------------------------------
struct ResourceClaim {
    int   grubId     = -1;
    int   resourceId = -1;
    bool  isWater    = false;
    float claimTime  = 0.0f;
};

// ------------------------------------------------------------
// Simulation core class
// Owns all simulation state and update logic
// ------------------------------------------------------------
class Simulation {
public:
    // ---------------- Environment ----------------
    int   envW = 0;
    int   envH = 0;
    float timeScale = 1.0f;

    bool paused = false;
    bool debugDraw = false;
    bool simulationEnded = false;
    bool userStopped = false;

    float totalTime = 0.0f;

    // ---------------- Core entities ----------------
    std::vector<Grub>     grubs;
    std::vector<Resource> foods;
    std::vector<Resource> waters;

    // ---------------- Spatial grids ----------------
    UniformGrid foodGrid;
    UniformGrid waterGrid;
    UniformGrid grubGrid;

    // ---------------- User-configurable parameters ----------------
    int   initialGrubs  = Config::DEFAULT_GRUBS;
    int   initialFood   = Config::DEFAULT_FOOD;
    int   initialWater  = Config::DEFAULT_WATER;

    float baseMutationChance = Config::DEFAULT_MUTATION_CHANCE;
    float mutationScale      = 25.0f;

    // Reproduction controls
    float userCrowdingFactorC    = 0.0f; // 0 = auto
    float baseReproductionRateB  = Config::BASE_REPRO_B;
    bool  temperatureEnabled    = false;

    // ---------------- Statistics ----------------
    struct SampleData {
        float time        = 0.0f;
        float avgSpeed    = 0.0f;
        float avgSight    = 0.0f;
        float avgMetab    = 0.0f;
        float avgMutation = 0.0f;
        int   population  = 0;
    };

    std::vector<SampleData> samples;

    // ---------------- Lifecycle ----------------
    void Init(int width, int height);
    void UpdateFixedStep();
    void Draw() const;

    void EndSimulation();
    void StopSimulation();
    bool ShouldEndSimulation() const;

    // ---------------- Configuration / Queries ----------------
    void  SetConfiguration(float crowdingC,
                           float reproductionB,
                           bool tempEnabled = false);

    int   CalculateCrowdingFactor() const;
    float GetCurrentReproductionChance() const;

    void  SaveDataToFile(const std::string& filename) const;

    // ---------------- UI-safe getters ----------------
    int   GetGrubCount()  const { return (int)grubs.size(); }
    int   GetFoodCount()  const { return (int)foods.size(); }
    int   GetWaterCount() const { return (int)waters.size(); }

    float GetAverageSpeed() const;
    float GetAverageSight() const;
    void DrawGrubTooltip(Vector2 mousePos) const;
    
    // Add these getter methods:
    float GetTotalTime() const { return totalTime; }
    bool IsPaused() const { return paused; }
    bool IsDebugDraw() const { return debugDraw; }
    float GetTimeScale() const { return timeScale; }
    bool IsSimulationEnded() const { return simulationEnded; }
    bool IsUserStopped() const { return userStopped; }
    // Add these setter methods:
    void SetPaused(bool p) { paused = p; }
    void SetDebugDraw(bool d) { debugDraw = d; }
    void SetTimeScale(float ts) { timeScale = ts; }

private:
    // ---------------- Resource claim system ----------------
    std::vector<ResourceClaim> resourceClaims;

    int   FindClaimedGrub(int resourceIdx, bool isWater);
    void  ClaimResource(int grubIdx, int resourceIdx, bool isWater);
    void  ClearExpiredClaims(float currentTime);
    void  ClearResourceClaim(int resourceIdx, bool isWater);

    // ---------------- Internal timing ----------------
    float reproduceTimer = 0.0f;
    float sampleTimer    = 0.0f;
    int   gridRebuildCounter = 0;

    // ---------------- Competition and fitness system ----------------
    float competitionFactor = 1.0f;
    
    // NEW METHODS: Add these declarations
    float CalculateFitness(const Grub& g) const;
    float CalculatePopulationFitness() const;
    void  ApplyNaturalSelection();
    int   FindMateByFitness(const Grub& g, float mateRange);

    // ---------------- Internal helpers ----------------
    void     RebuildGrids();
    Resource* FindBestResource(const Grub& g);
    void     AttemptReproductionFixedTick();
    void     RemoveDeadGrubs();
    void     SampleStatistics();
    float    InheritTraitAverage(float p1, float p2,
                                 float mutationChance,
                                 float mutationScale,
                                 float minV, float maxV);

    void     CalculateAverages(float& avgSpeed,
                               float& avgSight) const;
    const Grub* GetGrubAtPosition(Vector2 position) const;
};