// Simulation.h
#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include "Grub.h"
#include "Resource.h"
#include "UniformGrid.h"
#include "Config.h"

// Resource claim structure for tracking which grub is targeting which resource
struct ResourceClaim {
    int grubId = -1;
    int resourceId = -1;
    bool isWater = false;
    float claimTime = 0.0f;
};

class Simulation {
    
private:
    // Resource claiming system
    std::vector<ResourceClaim> resourceClaims;
    
    // Internal timing
    float reproduceTimer = 0.0f;
    float sampleTimer = 0.0f;
    int gridRebuildCounter = 0;

    // Internal methods
    void RebuildGrids();
    Resource* FindBestResource(const Grub &g);
    void AttemptReproductionFixedTick();
    void RemoveDeadGrubs();
    void SampleStatistics();
    float InheritTraitAverage(float p1, float p2, float mutationChance, 
                             float mutationScale, float minV, float maxV);
    
    // Resource claim methods
    int FindClaimedGrub(int resourceIdx, bool isWater);
    void ClaimResource(int grubIdx, int resourceIdx, bool isWater);
    void ClearExpiredClaims(float currentTime);
    void ClearResourceClaim(int resourceIdx, bool isWater);
    
    // Helper for average calculations
    void CalculateAverages(float& avgSpeed, float& avgSight) const;
    
public:
    int envW, envH;
    float timeScale = 1.0f;
    bool paused = false;
    bool debugDraw = false;
    bool simulationEnded = false;
    bool userStopped = false;
    
    float totalTime = 0.0f;

    // Core simulation objects
    std::vector<Grub> grubs;
    std::vector<Resource> foods;
    std::vector<Resource> waters;

    // Spatial partitioning
    UniformGrid foodGrid;
    UniformGrid waterGrid;
    UniformGrid grubGrid;

    // User-configurable parameters
    int initialGrubs = Config::DEFAULT_GRUBS;
    int initialFood = Config::DEFAULT_FOOD;
    int initialWater = Config::DEFAULT_WATER;
    float baseMutationChance = Config::DEFAULT_MUTATION_CHANCE;
    float mutationScale = 25.0f;
    
    // User-defined crowding factor and reproduction rate
    float userCrowdingFactorC = 0.0f; // 0 means use auto-calculated
    float baseReproductionRateB = Config::BASE_REPRO_B;
    bool temperatureEnabled = false;
    
    int CalculateCrowdingFactor() const;
    
    // Configuration setter
    void SetConfiguration(float crowdingC, float reproductionB, bool tempEnabled = false);
    
    // Get current reproduction chance for UI
    float GetCurrentReproductionChance() const;
    
    // Statistics
    struct SampleData {
        float time;
        float avgSpeed;
        float avgSight;
        float avgMetab;
        float avgMutation;
        int population;
    };
    std::vector<SampleData> samples;
    // Methods
    void Init(int width, int height);
    void UpdateFixedStep();
    void Draw() const;
    void EndSimulation();
    void StopSimulation();
    bool ShouldEndSimulation() const;
    void SaveDataToFile(const std::string& filename) const;
    
    // Getter methods for UI
    int GetGrubCount() const { return (int)grubs.size(); }
    int GetFoodCount() const { return (int)foods.size(); }
    int GetWaterCount() const { return (int)waters.size(); }
    float GetAverageSpeed() const;
    float GetAverageSight() const;
};