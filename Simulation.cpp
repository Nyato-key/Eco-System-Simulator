// Simulation.cpp
#include "Simulation.h"
#include "Grub.h"
#include "Utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <raymath.h>
#include <numeric>
#include <unordered_set>


// HELPER FUNCTIONS
// Helper methods for resource claims
int Simulation::FindClaimedGrub(int resourceIdx, bool isWater) {
    for (const auto& claim : resourceClaims) {
        if (claim.resourceId == resourceIdx && claim.isWater == isWater) {
            return claim.grubId;
        }
    }
    return -1;
}

void Simulation::ClaimResource(int grubIdx, int resourceIdx, bool isWater) {
    // Clear any existing claim for this resource
    ClearResourceClaim(resourceIdx, isWater);
    
    // Create new claim
    ResourceClaim claim;
    claim.grubId = grubIdx;
    claim.resourceId = resourceIdx;
    claim.isWater = isWater;
    claim.claimTime = totalTime;
    resourceClaims.push_back(claim);
}

void Simulation::ClearExpiredClaims(float currentTime) {
    // Remove claims older than 2 seconds
    resourceClaims.erase(
        std::remove_if(resourceClaims.begin(), resourceClaims.end(),
            [currentTime](const ResourceClaim& claim) {
                return currentTime - claim.claimTime > 2.0f;
            }),
        resourceClaims.end()
    );
}

void Simulation::ClearResourceClaim(int resourceIdx, bool isWater) {
    resourceClaims.erase(
        std::remove_if(resourceClaims.begin(), resourceClaims.end(),
            [resourceIdx, isWater](const ResourceClaim& claim) {
                return claim.resourceId == resourceIdx && claim.isWater == isWater;
            }),
        resourceClaims.end()
    );
}

// ==================== CONFIGURATION ====================

void Simulation::SetConfiguration(float crowdingC, float reproductionB, bool tempEnabled) {
    userCrowdingFactorC = crowdingC;
    baseReproductionRateB = reproductionB;
    temperatureEnabled = tempEnabled;
}

float Simulation::GetCurrentReproductionChance() const {
    int N = (int)grubs.size();
    int C = (userCrowdingFactorC > 0) ? 
            (int)userCrowdingFactorC : 
            CalculateCrowdingFactor();
    
    if (C <= 0) return baseReproductionRateB;
    
    float effective = baseReproductionRateB * (1.0f - (float)N / (float)C);
    return clampVal(effective, 0.0f, baseReproductionRateB);
}

int Simulation::CalculateCrowdingFactor() const {
    float area = envW * envH;
    int crowding = static_cast<int>(area / Config::AREA_PER_GRUB);
    return std::max(10, crowding);
}

// ==================== FITNESS CALCULATION ====================

float Simulation::CalculateFitness(const Grub& g) const {
    // Fitness formula: weighted combination of traits
    // Speed: 40% weight, Sight: 40% weight, Metabolism: 20% weight (lower is better)
    float speedScore = (g.speed - Config::SPEED_MIN) / (Config::SPEED_MAX - Config::SPEED_MIN);
    float sightScore = (g.sight - Config::SIGHT_MIN) / (Config::SIGHT_MAX - Config::SIGHT_MIN);
    float metabScore = 1.0f - (g.metabolism - Config::METAB_MIN) / (Config::METAB_MAX - Config::METAB_MIN);
    
    return (speedScore * 0.4f + sightScore * 0.4f + metabScore * 0.2f);
}
float Simulation::CalculatePopulationFitness() const {
    if (grubs.empty()) return 0.5f;
    
    float totalFitness = 0.0f;
    int count = 0;
    
    for (const auto& g : grubs) {
        if (g.alive) {
            float fitness = CalculateFitness(g);
            // Check for valid fitness
            if (!std::isnan(fitness) && !std::isinf(fitness)) {
                totalFitness += fitness;
                count++;
            }
        }
    }
    
    if (count == 0) return 0.5f;
    
    float avgFitness = totalFitness / count;
    
    // Ensure valid range
    if (std::isnan(avgFitness) || std::isinf(avgFitness)) {
        return 0.5f;
    }
    
    return clampVal(avgFitness, 0.0f, 1.0f);
}
// ==================== TRAIT-BASED ADVANTAGES ====================

Resource* Simulation::FindBestResource(const Grub& g) {
    Resource* bestResource = nullptr;
    
    // TRAIT ADVANTAGE: Grubs with higher sight can detect resources from farther effectively
    float effectiveSight = g.sight * (1.0f + (g.sight - Config::SIGHT_MIN) / 
                        (Config::SIGHT_MAX - Config::SIGHT_MIN) * 0.3f);
    
    float bestDistance = effectiveSight;  // Use effective sight, not base sight
    bool preferWater = g.thirst < g.hunger;

    // Check preferred resource type first
    auto& resources = preferWater ? waters : foods;
    auto& grid = preferWater ? waterGrid : foodGrid;
    
    std::vector<int> candidates;
    grid.CollectCandidates(g.position.x, g.position.y, effectiveSight, candidates);
    
    for (int idx : candidates) {
        if (idx < 0 || idx >= (int)resources.size()) continue;
        Resource &r = resources[idx];
        if (!r.active) continue;
        
        float dist = Vector2Distance(g.position, r.position);
        if (dist < bestDistance) {
            bestDistance = dist;
            bestResource = &r;
        }
    }
    
    // If no preferred resource found, try the other type
    if (!bestResource) {
        auto& otherResources = preferWater ? foods : waters;
        auto& otherGrid = preferWater ? foodGrid : waterGrid;
        
        candidates.clear();
        otherGrid.CollectCandidates(g.position.x, g.position.y, effectiveSight, candidates);
        
        for (int idx : candidates) {
            if (idx < 0 || idx >= (int)otherResources.size()) continue;
            Resource &r = otherResources[idx];
            if (!r.active) continue;
            
            float dist = Vector2Distance(g.position, r.position);
            if (dist < bestDistance) {
                bestDistance = dist;
                bestResource = &r;
            }
        }
    }
    
    return bestResource;
}

// ==================== NATURAL SELECTION ====================

void Simulation::ApplyNaturalSelection() {
    if (grubs.size() <= 10) return;  // Don't apply selection if population is too small
    
    // Calculate survival rate based on resource availability
    int activeFood = 0, activeWater = 0;
    for (const auto& f : foods) if (f.active) activeFood++;
    for (const auto& w : waters) if (w.active) activeWater++;
    
    int totalResources = activeFood + activeWater;
    if (totalResources == 0) totalResources = 1;
    
    // Survival rate: more resources = higher survival rate
    float survivalRate = clampVal((float)totalResources / (float)grubs.size(), 0.3f, 0.9f);
    
    // Calculate fitness scores for all alive grubs
    std::vector<std::pair<float, int>> fitnessScores;
    for (int i = 0; i < (int)grubs.size(); ++i) {
        if (grubs[i].alive) {
            float fitness = CalculateFitness(grubs[i]);
            fitnessScores.push_back({fitness, i});
        }
    }
    
    if (fitnessScores.empty()) return;
    
    // Sort by fitness (highest first)
    std::sort(fitnessScores.begin(), fitnessScores.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Kill weakest based on survival rate
    int survivorsToKeep = (int)(fitnessScores.size() * survivalRate);
    for (int i = survivorsToKeep; i < (int)fitnessScores.size(); ++i) {
        grubs[fitnessScores[i].second].alive = false;
    }
}

// ==================== SEXUAL SELECTION ====================

int Simulation::FindMateByFitness(const Grub& g, float mateRange) {
    std::vector<std::pair<float, int>> potentialMates;
    
    for (int i = 0; i < (int)grubs.size(); ++i) {
        if (!grubs[i].alive || !grubs[i].CanReproduce()) continue;
        if (&grubs[i] == &g) continue;  // Skip self
        
        float distance = Vector2Distance(g.position, grubs[i].position);
        if (distance > mateRange) continue;
        
        // Calculate mate attractiveness based on fitness and distance
        float mateFitness = CalculateFitness(grubs[i]);
        
        // Combine fitness and proximity (closer mates are more attractive)
        float attractiveness = mateFitness / (1.0f + distance * 0.1f);
        potentialMates.push_back({attractiveness, i});
    }
    
    if (potentialMates.empty()) return -1;
    
    // Sort by attractiveness
    std::sort(potentialMates.begin(), potentialMates.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Choose from top 3 most attractive mates
    int topCount = std::min(3, (int)potentialMates.size());
    int chosenIndex = randInt(0, topCount - 1);
    return potentialMates[chosenIndex].second;
}

// ==================== INHERITANCE & MUTATION ====================

float Simulation::InheritTraitAverage(float p1, float p2, float mutationProbability, 
                                     float mutationScale, float minV, float maxV) {
    // Randomly inherit from either parent (50/50) instead of average
    float child = (randFloat(0.0f, 1.0f) < 0.5f) ? p1 : p2;
    
    if (randFloat(0.0f, 1.0f) < mutationProbability) {
        float mutation = randFloat(-1.0f, 1.0f) * mutationScale;
        child += mutation;
    }
    return clampVal(child, minV, maxV);
}

// ==================== REPRODUCTION ====================

void Simulation::AttemptReproductionFixedTick() {
    reproduceTimer += Config::FIXED_STEP * timeScale;
    if (reproduceTimer < Config::REPRODUCE_INTERVAL) return;
    reproduceTimer = 0.0f;

    int N = (int)grubs.size();
    if (N >= Config::MAX_GRUBS) return;

    // Calculate competition factor (population vs resources)
    if (grubs.empty()) {
        competitionFactor = 1.0f;
    } else {
        int activeFood = 0, activeWater = 0;
        for (const auto& f : foods) if (f.active) activeFood++;
        for (const auto& w : waters) if (w.active) activeWater++;

        int totalResources = activeFood + activeWater;
        if (totalResources == 0) totalResources = 1;

        float resourceRatio = (float)totalResources / (float)grubs.size();
        float RCI = (Config::TARGET_RESOURCE_RATIO - resourceRatio) / Config::TARGET_RESOURCE_RATIO;
        competitionFactor = clampVal(1.0f + RCI * Config::BASE_COMPETITION_FACTOR,
                                    Config::MIN_MUTATION_SCALE, Config::MAX_MUTATION_SCALE);
    }

    int crowdingC = (userCrowdingFactorC > 0) ? (int)userCrowdingFactorC : CalculateCrowdingFactor();
    float effectiveFactor = clampVal(1.0f - float(N) / float(crowdingC), 0.0f, 1.0f);
    float effectiveB = baseReproductionRateB * effectiveFactor;

    // Calculate population fitness for adaptive mutation
    float populationFitness = CalculatePopulationFitness();
    
    // ADAPTIVE MUTATION: When population is fit, mutations are smaller/refined
    // When population is struggling, mutations are larger (more radical)
    float fitnessPressure = 1.0f - populationFitness;  // 0 = very fit, 1 = struggling
    float adaptiveMutationBoost = 1.0f + fitnessPressure * 0.5f;

    std::vector<Grub> newBabies;
    newBabies.reserve(32);

    for (int i = 0; i < (int)grubs.size(); ++i) {
        Grub &g = grubs[i];
        if (!g.CanReproduce()) continue;
        if (randFloat(0.0f, 1.0f) >= effectiveB) continue;

        // Find mate using fitness-based selection
        float mateRange = g.sight * Config::MATING_RANGE_FACTOR;
        int mateIdx = FindMateByFitness(g, mateRange);
        
        if (mateIdx < 0) continue;

        Grub &p1 = g;
        Grub &p2 = grubs[mateIdx];
        float avgMutChance = (p1.mutationChance + p2.mutationChance) * 0.5f;
        float mutationProbability = avgMutChance / 100.0f;

        // Inherit traits with competition and adaptive mutation
        float speedMutationScale = 0.15f * (Config::SPEED_MAX - Config::SPEED_MIN) * 
                                  competitionFactor * adaptiveMutationBoost;
        float sightMutationScale = 0.2f * (Config::SIGHT_MAX - Config::SIGHT_MIN) * 
                                 competitionFactor * adaptiveMutationBoost;
        float metabMutationScale = 0.01f * (Config::METAB_MAX - Config::METAB_MIN) * 
                                 competitionFactor * adaptiveMutationBoost;
        float mutChanceMutationScale = 2.0f * competitionFactor * adaptiveMutationBoost;

        float childSpeed = InheritTraitAverage(p1.speed, p2.speed, mutationProbability,
                                               speedMutationScale,
                                               Config::SPEED_MIN, Config::SPEED_MAX);

        float childSight = InheritTraitAverage(p1.sight, p2.sight, mutationProbability,
                                               sightMutationScale,
                                               Config::SIGHT_MIN, Config::SIGHT_MAX);

        float childMetab = InheritTraitAverage(p1.metabolism, p2.metabolism, mutationProbability,
                                               metabMutationScale,
                                               Config::METAB_MIN, Config::METAB_MAX);

        float childMutChance = InheritTraitAverage(p1.mutationChance, p2.mutationChance, 
                                                   mutationProbability,
                                                   mutChanceMutationScale,
                                                   Config::MUTATION_CHANCE_MIN, 
                                                   Config::MUTATION_CHANCE_MAX);

        Vector2 childPos = {
            (p1.position.x + p2.position.x) * 0.5f + randFloat(-10.0f, 10.0f),
            (p1.position.y + p2.position.y) * 0.5f + randFloat(-10.0f, 10.0f)
        };

        newBabies.emplace_back(childPos, childSpeed, childSight, childMetab, childMutChance);

        // Apply parent reproduction cost
        p1.StartReproCooldown();
        p2.StartReproCooldown();
        p1.hunger = clampVal(p1.hunger - Config::REPRODUCTION_ENERGY_COST, 0.0f, 1.0f);
        p2.hunger = clampVal(p2.hunger - Config::REPRODUCTION_ENERGY_COST, 0.0f, 1.0f);
        p1.thirst = clampVal(p1.thirst - Config::REPRODUCTION_ENERGY_COST, 0.0f, 1.0f);
        p2.thirst = clampVal(p2.thirst - Config::REPRODUCTION_ENERGY_COST, 0.0f, 1.0f);
    }

    // Add babies under population cap
    if (grubs.size() + newBabies.size() <= Config::MAX_GRUBS) {
        grubs.insert(grubs.end(), newBabies.begin(), newBabies.end());
    }
}

// ==================== INITIALIZATION ====================

void Simulation::Init(int width, int height) {
    envW = width; 
    envH = height;
    
    // Clear resource claims
    resourceClaims.clear();
    
    // Initialize grids
    float cellSize = Config::GRID_CELL_SIZE;
    foodGrid.Init(envW, envH, cellSize);
    waterGrid.Init(envW, envH, cellSize);
    grubGrid.Init(envW, envH, cellSize);
    
    // Clear existing data
    grubs.clear(); 
    foods.clear(); 
    waters.clear();
    samples.clear();
    
    grubs.reserve(Config::MAX_GRUBS);
    foods.reserve(initialFood * 2);
    waters.reserve(initialWater * 2);

    // Create foods
    for (int i = 0; i < initialFood; ++i) {
        Resource r; 
        r.isWater = false; 
        r.value = Config::FOOD_VALUE;
        r.active = true;
        r.Respawn(envW, envH);
        foods.push_back(r);
    }

    // Create waters
    for (int i = 0; i < initialWater; ++i) {
        Resource r; 
        r.isWater = true; 
        r.value = Config::WATER_VALUE;
        r.active = true;
        r.Respawn(envW, envH);
        waters.push_back(r);
    }
    
    // Create grubs with genetic diversity
    for (int i = 0; i < initialGrubs; ++i) {
        Vector2 p = { randFloat(40.0f, envW - 40.0f), randFloat(40.0f, envH - 40.0f) };
        
        // Use full trait range for initial genetic diversity
        float speed = randFloat(Config::SPEED_MIN, Config::SPEED_MAX);
        float sight = randFloat(Config::SIGHT_MIN, Config::SIGHT_MAX);
        float metab = randFloat(Config::METAB_MIN, Config::METAB_MAX);
        
        Grub g(p, speed, sight, metab, baseMutationChance);
        grubs.push_back(g);
    }
    
    // Reset timers and state
    reproduceTimer = 0.0f;
    sampleTimer = 0.0f;
    totalTime = 0.0f;
    gridRebuildCounter = 0;
    simulationEnded = false;
    userStopped = false;
    competitionFactor = 1.0f;
}

// ==================== GRID MANAGEMENT ====================

void Simulation::RebuildGrids() {
    foodGrid.Clear();
    waterGrid.Clear();
    grubGrid.Clear();
    
    for (int i = 0; i < (int)foods.size(); ++i) 
        if (foods[i].active) foodGrid.Insert(i, foods[i].position.x, foods[i].position.y);
            
    for (int i = 0; i < (int)waters.size(); ++i) 
        if (waters[i].active) waterGrid.Insert(i, waters[i].position.x, waters[i].position.y);
            
    for (int i = 0; i < (int)grubs.size(); ++i) 
        if (grubs[i].alive) grubGrid.Insert(i, grubs[i].position.x, grubs[i].position.y);
}

// ==================== CLEANUP ====================

void Simulation::RemoveDeadGrubs() {
    grubs.erase(std::remove_if(grubs.begin(), grubs.end(), 
        [](const Grub& g) { return !g.alive; }), grubs.end());
}

// ==================== STATISTICS ====================

void Simulation::SampleStatistics() {
    sampleTimer += Config::FIXED_STEP * timeScale;
    if (sampleTimer >= Config::SAMPLE_INTERVAL) {
        sampleTimer = 0.0f;
        
        float avgSpeed = 0.0f, avgSight = 0.0f, avgMetab = 0.0f, avgMutation = 0.0f;
        int count = 0;
        
        for (auto &g : grubs) {
            if (g.alive) {
                avgSpeed += g.speed;
                avgSight += g.sight;
                avgMetab += g.metabolism;
                avgMutation += g.mutationChance;
                count++;
            }
        }
        
        if (count > 0) {
            avgSpeed /= count;
            avgSight /= count;
            avgMetab /= count;
            avgMutation /= count;
        }
        
        SampleData data;
        data.time = totalTime;
        data.avgSpeed = avgSpeed;
        data.avgSight = avgSight;
        data.avgMetab = avgMetab;
        data.avgMutation = avgMutation;
        data.population = count;
        
        samples.push_back(data);
    }
}

void Simulation::CalculateAverages(float& avgSpeed, float& avgSight) const {
    avgSpeed = 0.0f;
    avgSight = 0.0f;
    int count = 0;
    
    for (const auto& g : grubs) {
        if (g.alive) {
            avgSpeed += g.speed;
            avgSight += g.sight;
            count++;
        }
    }
    
    if (count > 0) {
        avgSpeed /= count;
        avgSight /= count;
    }
}

float Simulation::GetAverageSpeed() const {
    float avgSpeed, avgSight;
    CalculateAverages(avgSpeed, avgSight);
    return avgSpeed;
}

float Simulation::GetAverageSight() const {
    float avgSpeed, avgSight;
    CalculateAverages(avgSpeed, avgSight);
    return avgSight;
}

// ==================== MAIN UPDATE LOOP ====================

void Simulation::UpdateFixedStep() {
    if (simulationEnded) return;
    
    totalTime += Config::FIXED_STEP * timeScale;
    
    // Apply natural selection every 10 seconds
    static float selectionTimer = 0.0f;
    selectionTimer += Config::FIXED_STEP * timeScale;
    if (selectionTimer >= 10.0f) {
        ApplyNaturalSelection();
        selectionTimer = 0.0f;
    }
    
    // Clear expired claims
    ClearExpiredClaims(totalTime);
    
    // Rebuild grids less frequently (optimization)
    gridRebuildCounter++;
    if (gridRebuildCounter >= 4) {
        RebuildGrids();
        gridRebuildCounter = 0;
    }

    // First pass: Find resources for each grub
    std::vector<int> grubTargetResource(grubs.size(), -1);
    std::vector<bool> grubTargetIsWater(grubs.size(), false);
    
    for (int i = 0; i < (int)grubs.size(); ++i) {
        if (!grubs[i].alive) continue;
        
        Grub &g = grubs[i];
        
        // Only look for resources if needed
        if (g.NeedsFoodOrWater()) {
            Resource* bestResource = FindBestResource(g);
            if (bestResource) {
                // Calculate resource index
                int resourceIdx;
                bool isWater = bestResource->isWater;
                
                if (isWater) {
                    resourceIdx = bestResource - &waters[0];
                } else {
                    resourceIdx = bestResource - &foods[0];
                }
                
                // Check if resource is already claimed
                int claimedBy = FindClaimedGrub(resourceIdx, isWater);
                
                if (claimedBy == -1 || claimedBy == i) {
                    // Resource available or claimed by this grub
                    grubTargetResource[i] = resourceIdx;
                    grubTargetIsWater[i] = isWater;
                    
                    // Claim the resource
                    ClaimResource(i, resourceIdx, isWater);
                }
            }
        }
    }

    // Second pass: Update grub movement
    for (int i = 0; i < (int)grubs.size(); ++i) {
        if (!grubs[i].alive) continue;
        Grub &g = grubs[i];
        
        // Move toward resource if has target
        if (grubTargetResource[i] != -1) {
            Vector2 targetPos;
            if (grubTargetIsWater[i]) {
                targetPos = waters[grubTargetResource[i]].position;
            } else {
                targetPos = foods[grubTargetResource[i]].position;
            }
            
            // TRAIT ADVANTAGE: Faster grubs move more efficiently toward targets
            float moveEfficiency = 1.0f + (g.speed - Config::SPEED_MIN) / 
                                 (Config::SPEED_MAX - Config::SPEED_MIN) * 0.2f;
            
            g.MoveTowards(targetPos, Config::FIXED_STEP * timeScale * moveEfficiency);
            
            // Clear any random target when going to resource
            g.ClearTarget();
        } else {
            // Use wander system (random point at sight radius)
            g.Wander(Config::FIXED_STEP * timeScale, envW, envH);
        }

        // Check collisions with resources
        if (g.NeedsFoodOrWater()) {
            // Check food if not specifically targeting water
            if (grubTargetIsWater[i] != true) {
                std::vector<int> foodCandidates;
                foodGrid.CollectCandidates(g.position.x, g.position.y, 
                          g.size + Config::FOOD_RADIUS + g.speed * 0.1f,
                          foodCandidates);
                
                for (int idx : foodCandidates) {
                    if (idx < 0 || idx >= (int)foods.size()) continue;
                    Resource &food = foods[idx];
                    if (!food.active) continue;
                    
                    float dist = Vector2Distance(g.position, food.position);
                    if (dist < g.size + Config::FOOD_RADIUS) {
                        g.Feed(food.value);
                        food.Respawn(envW, envH);
                        ClearResourceClaim(idx, false);
                        break;
                    }
                }
            }
            
            // Check water if not specifically targeting food
            if (grubTargetIsWater[i] != false) {
                std::vector<int> waterCandidates;
                waterGrid.CollectCandidates(g.position.x, g.position.y, 
                                           g.size + Config::WATER_RADIUS, waterCandidates);
                
                for (int idx : waterCandidates) {
                    if (idx < 0 || idx >= (int)waters.size()) continue;
                    Resource &water = waters[idx];
                    if (!water.active) continue;
                    
                    float dist = Vector2Distance(g.position, water.position);
                    if (dist < g.size + Config::WATER_RADIUS) {
                        g.Drink(water.value);
                        water.Respawn(envW, envH);
                        ClearResourceClaim(idx, true);
                        break;
                    }
                }
            }
        }

        g.ApplyBounds(envW, envH);
        g.UpdateBars(Config::FIXED_STEP * timeScale);
    }

    RemoveDeadGrubs();
    AttemptReproductionFixedTick();
    SampleStatistics();
    
    if (ShouldEndSimulation()) {
        EndSimulation();
    }
}

// ==================== SIMULATION CONTROL ====================

bool Simulation::ShouldEndSimulation() const {
    return grubs.empty() || totalTime > 600.0f || userStopped;
}

void Simulation::EndSimulation() {
    simulationEnded = true;
}

void Simulation::StopSimulation() {
    userStopped = true;
    simulationEnded = true;
}
// Add these methods to Simulation.cpp:

// Helper: Check if point is within grub's radius
const Grub* Simulation::GetGrubAtPosition(Vector2 position) const {
    for (const auto& grub : grubs) {
        if (!grub.alive) continue;
        
        float distance = Vector2Distance(position, grub.position);
        if (distance <= grub.size * 1.5f) {  // 1.5x size for easier hovering
            return &grub;
        }
    }
    return nullptr;
}

// Draw tooltip for hovered grub
void Simulation::DrawGrubTooltip(Vector2 mousePos) const {
    const Grub* hoveredGrub = GetGrubAtPosition(mousePos);
    if (!hoveredGrub) return;
    
    // Tooltip dimensions and position
    float tooltipWidth = 300.0f;
    float tooltipHeight = 180.0f;
    
    // Position tooltip to the right of mouse, but adjust if near screen edge
    float x = mousePos.x + 20.0f;
    float y = mousePos.y - tooltipHeight / 2.0f;
    
    // Adjust if tooltip would go off screen
    if (x + tooltipWidth > envW) {
        x = mousePos.x - tooltipWidth - 20.0f;  // Show to the left
    }
    if (y < 0) y = 0;
    if (y + tooltipHeight > envH) y = envH - tooltipHeight;
    
    // Draw tooltip background
    DrawRectangleRounded(Rectangle{x, y, tooltipWidth, tooltipHeight}, 0.1f, 8, Color{0, 0, 0, 220});
    DrawRectangleRoundedLines(Rectangle{x, y, tooltipWidth, tooltipHeight}, 0.1f, 8, LIME);    
    // Draw title
    DrawText("Grub Details", x + 10, y + 10, 18, LIME);
    
    // Draw stats in two columns
    float startY = y + 40;
    float col1X = x + 15;
    float col2X = x + 130;
    
    // Column 1: Basic stats
    DrawText(TextFormat("Speed: %.1f", hoveredGrub->GetSpeed()), col1X, startY, 16, WHITE);
    DrawText(TextFormat("Sight: %.1f", hoveredGrub->GetSight()), col1X, startY + 22, 16, WHITE);
    DrawText(TextFormat("Metabolism: %.3f", hoveredGrub->GetMetabolism()), col1X, startY + 44, 16, WHITE);
    DrawText(TextFormat("Mutation: %.1f%%", hoveredGrub->GetMutationChance()), col1X, startY + 66, 16, WHITE);
    
    // Column 2: State info
    DrawText(hoveredGrub->IsReproductive() ? "Ready to mate" : "Not reproductive", 
             col2X, startY, 16, hoveredGrub->IsReproductive() ? YELLOW : GRAY);
    
    if (hoveredGrub->GetReproCooldown() > 0) {
        DrawText(TextFormat("Cooldown: %.1fs", hoveredGrub->GetReproCooldown()), 
                 col2X, startY + 22, 14, ORANGE);
    }
    
    // Draw hunger and thirst bars
    float barY = y + tooltipHeight - 45;
    float barWidth = tooltipWidth - 20;
    
    // Hunger bar
    DrawText("Hunger:", x + 10, barY, 16, WHITE);
    float hunger = hoveredGrub->GetHunger();
    DrawRectangle(x + 70, barY + 2, barWidth - 70, 12, Color{40, 40, 40, 255});
    DrawRectangle(x + 70, barY + 2, (barWidth - 70) * hunger, 12, 
                 hunger > 0.5f ? GREEN : hunger > 0.25f ? YELLOW : RED);
    DrawText(TextFormat("%.0f%%", hunger * 100), x + barWidth - 40, barY, 14, WHITE);
    
    // Thirst bar
    DrawText("Thirst:", x + 10, barY + 18, 16, WHITE);
    float thirst = hoveredGrub->GetThirst();
    DrawRectangle(x + 70, barY + 20, barWidth - 70, 12, Color{40, 40, 40, 255});
    DrawRectangle(x + 70, barY + 20, (barWidth - 70) * thirst, 12, 
                 thirst > 0.5f ? SKYBLUE : thirst > 0.25f ? BLUE : DARKBLUE);
    DrawText(TextFormat("%.0f%%", thirst * 100), x + barWidth - 40, barY + 18, 14, WHITE);
    
    // Draw a small highlight circle around the hovered grub
    DrawCircleLines(hoveredGrub->position.x, hoveredGrub->position.y, 
                   hoveredGrub->size + 5, YELLOW);
}
// ==================== DATA SAVING ====================

void Simulation::SaveDataToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not save data to file: " << filename << std::endl;
        return;
    }
    
    // Enhanced CSV header with fitness data
    file << "Time(seconds),Population,AverageSpeed,AverageSight,AverageMetabolism,AverageMutationRate(%),CompetitionFactor,PopulationFitness\n";
    
    // Write all sample data
    for (const auto& sample : samples) {
        file << sample.time << ","
             << sample.population << ","
             << sample.avgSpeed << ","
             << sample.avgSight << ","
             << sample.avgMetab << ","
             << sample.avgMutation << ","
             << competitionFactor << ","
             << CalculatePopulationFitness() << "\n";
    }
    
    file.close();
    std::cout << "Simulation data saved to: " << filename << std::endl;
    std::cout << "Total samples recorded: " << samples.size() << std::endl;
    
    if (!samples.empty()) {
        std::cout << "Simulation duration: " << samples.back().time << " seconds" << std::endl;
        std::cout << "Final population: " << samples.back().population << " grubs" << std::endl;
        std::cout << "Final average fitness: " << CalculatePopulationFitness() << std::endl;
    }
}


// ==================== DRAWING ====================


void Simulation::Draw() const {
    // Draw resources
    for (auto &f : foods) f.Draw();
    for (auto &w : waters) w.Draw();
    
    // Draw grubs
    for (auto &g : grubs) g.Draw();
    
    // Debug overlays
    if (debugDraw) {
        int limit = std::min((int)grubs.size(), 20);
        for (int i = 0; i < limit; ++i) {
            const Grub &g = grubs[i];
            DrawCircleLines((int)g.position.x, (int)g.position.y, g.sight, Fade(WHITE, 0.03f));
        }
    }
    
    // NEW: Draw hover tooltip if simulation is paused
    if (paused) {
        Vector2 mousePos = GetMousePosition();
        DrawGrubTooltip(mousePos);
    }
}