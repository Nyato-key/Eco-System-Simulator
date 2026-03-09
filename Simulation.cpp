// Simulation.cpp
#include "Simulation.h"
#include "Utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <raymath.h>

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

// Configuration setter
void Simulation::SetConfiguration(float crowdingC, float reproductionB, bool tempEnabled) {
    userCrowdingFactorC = crowdingC;
    baseReproductionRateB = reproductionB;
    temperatureEnabled = tempEnabled;
}

// Get current reproduction chance
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

void Simulation::SaveDataToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not save data to file: " << filename << std::endl;
        return;
    }
    
    // Write CSV header
    file << "Time(seconds),Population,AverageSpeed,AverageSight,AverageMetabolism,AverageMutationRate(%)\n";
    
    // Write all sample data
    for (const auto& sample : samples) {
        file << sample.time << ","
             << sample.population << ","
             << sample.avgSpeed << ","
             << sample.avgSight << ","
             << sample.avgMetab << ","
             << sample.avgMutation << "\n";
    }
    
    file.close();
    std::cout << "Simulation data saved to: " << filename << std::endl;
    std::cout << "Total samples recorded: " << samples.size() << std::endl;
    
    if (!samples.empty()) {
        std::cout << "Simulation duration: " << samples.back().time << " seconds" << std::endl;
        std::cout << "Final population: " << samples.back().population << " grubs" << std::endl;
    }
}

void Simulation::StopSimulation() {
    userStopped = true;
    simulationEnded = true;
}

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
        r.value = 0.5f;
        r.active = true;
        r.Respawn(envW, envH);
        foods.push_back(r);
    }

    // Create waters
    for (int i = 0; i < initialWater; ++i) {
        Resource r; 
        r.isWater = true; 
        r.value = 0.5f;
        r.active = true;
        r.Respawn(envW, envH);
        waters.push_back(r);
    }
    
    // Create grubs

    for (int i = 0; i < initialGrubs; ++i) {
        Vector2 p = { randFloat(40.0f, envW - 40.0f), randFloat(40.0f, envH - 40.0f) };
        float s = randFloat(100.0f, 180.0f);
        float sight = randFloat(150.0f, 400.0f);
        float metab = randFloat(0.02f, 0.08f);
        Grub g(p, s, sight, metab, baseMutationChance);
        grubs.push_back(g);
    }
    
    // Reset timers
    reproduceTimer = 0.0f;
    sampleTimer = 0.0f;
    totalTime = 0.0f;
    gridRebuildCounter = 0;
    simulationEnded = false;
    userStopped = false;
}

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

Resource* Simulation::FindBestResource(const Grub &g) {
    Resource* bestResource = nullptr;
    float bestDistance = g.sight;
    bool preferWater = g.thirst < g.hunger;

    // Check preferred resource type first
    auto& resources = preferWater ? waters : foods;
    auto& grid = preferWater ? waterGrid : foodGrid;
    
    std::vector<int> candidates;
    grid.CollectCandidates(g.position.x, g.position.y, g.sight, candidates);
    
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
        otherGrid.CollectCandidates(g.position.x, g.position.y, g.sight, candidates);
        
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

void Simulation::RemoveDeadGrubs() {
    grubs.erase(std::remove_if(grubs.begin(), grubs.end(), 
        [](const Grub& g) { return !g.alive; }), grubs.end());
}

float Simulation::InheritTraitAverage(float p1, float p2, float mutationChance, 
                                     float mutationScale, float minV, float maxV) {
    float child = 0.5f * (p1 + p2);
    if (randFloat(0.0f, 1.0f) < mutationChance) {
        float mutation = randFloat(-1.0f, 1.0f) * mutationScale;
        child += mutation;
    }
    return clampVal(child, minV, maxV);
}

void Simulation::AttemptReproductionFixedTick() {
    reproduceTimer += Config::FIXED_STEP * timeScale;
    if (reproduceTimer < Config::REPRODUCE_INTERVAL) return;
    reproduceTimer = 0.0f;

    int N = (int)grubs.size();
    if (N >= Config::MAX_GRUBS) return;

    // Use user-defined C if available, otherwise auto-calculate
    int crowdingC = (userCrowdingFactorC > 0) ? 
                    (int)userCrowdingFactorC : 
                    CalculateCrowdingFactor();
    
    float effectiveFactor = clampVal(1.0f - float(N) / float(crowdingC), 0.0f, 1.0f);
    float effectiveB = baseReproductionRateB * effectiveFactor; // Use user-defined B

    std::vector<Grub> newBabies;
    newBabies.reserve(32);

    for (int i = 0; i < (int)grubs.size(); ++i) {
        Grub &g = grubs[i];
        if (!g.CanReproduce()) continue;
        if (randFloat(0.0f, 1.0f) >= effectiveB) continue;

        // Find mate
        std::vector<int> candidates;
        float mateRange = g.sight * Config::MATING_RANGE_FACTOR;
        grubGrid.CollectCandidates(g.position.x, g.position.y, mateRange, candidates);

        int mateIdx = -1;
        for (int cid : candidates) {
            if (cid == i) continue;
            if (!grubs[cid].CanReproduce()) continue;
            if (Vector2Distance(g.position, grubs[cid].position) <= mateRange) {
                mateIdx = cid;
                break;
            }
        }

        if (mateIdx < 0) continue;

        // Create child - Use average of parents
        Grub &p1 = g;
        Grub &p2 = grubs[mateIdx];
        
        // Average mutation chance (0-100 range)
        float avgMutChance = (p1.mutationChance + p2.mutationChance) * 0.5f;
        
        // Convert to probability for trait mutations (0-1 range)
        float mutationProbability = avgMutChance / 100.0f;

        float childSpeed = InheritTraitAverage(p1.speed, p2.speed, mutationProbability,
                                       // Adjust mutation scale for larger speed range
                                       0.15f * (Config::SPEED_MAX - Config::SPEED_MIN), 
                                       Config::SPEED_MIN, Config::SPEED_MAX);

        float childSight = InheritTraitAverage(p1.sight, p2.sight, mutationProbability,
                                            // INCREASE mutation scale for larger sight range
                                            0.2f * (Config::SIGHT_MAX - Config::SIGHT_MIN), 
                                            Config::SIGHT_MIN, Config::SIGHT_MAX);

        float childMetab = InheritTraitAverage(p1.metabolism, p2.metabolism, mutationProbability,
                                               0.01f * (Config::METAB_MAX - Config::METAB_MIN), 
                                               Config::METAB_MIN, Config::METAB_MAX);

        float childMutChance = InheritTraitAverage(p1.mutationChance, p2.mutationChance, mutationProbability,
                                                   2.0f, Config::MUTATION_CHANCE_MIN, Config::MUTATION_CHANCE_MAX);

        Vector2 childPos = {
            (p1.position.x + p2.position.x) * 0.5f + randFloat(-10.0f, 10.0f),
            (p1.position.y + p2.position.y) * 0.5f + randFloat(-10.0f, 10.0f)
        };

        Grub baby(childPos, childSpeed, childSight, childMetab, childMutChance);
        newBabies.push_back(baby);

        // Parent costs
        p1.StartReproCooldown();
        p2.StartReproCooldown();
        p1.hunger = clampVal(p1.hunger - 0.2f, 0.0f, 1.0f);
        p2.hunger = clampVal(p2.hunger - 0.2f, 0.0f, 1.0f);
        p1.thirst = clampVal(p1.thirst - 0.2f, 0.0f, 1.0f);
        p2.thirst = clampVal(p2.thirst - 0.2f, 0.0f, 1.0f);
    }

    // Add babies if under population cap
    if (grubs.size() + newBabies.size() <= Config::MAX_GRUBS) {
        for (auto &baby : newBabies) {
            grubs.push_back(baby);
        }
    }
}

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

void Simulation::UpdateFixedStep() {
    if (simulationEnded) return;
    
    totalTime += Config::FIXED_STEP * timeScale;
    
    // Clear expired claims
    ClearExpiredClaims(totalTime);
    
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
            g.MoveTowards(targetPos, Config::FIXED_STEP * timeScale);
            
            // Clear any random target when going to resource
            g.ClearTarget();
        } else {
            // Use wander system (random point at sight radius)
            g.Wander(Config::FIXED_STEP * timeScale);
        }

        // Check collisions with resources
        if (g.NeedsFoodOrWater()) {
            // Check food if not specifically targeting water
            if (grubTargetIsWater[i] != true) {
                std::vector<int> foodCandidates;
                foodGrid.CollectCandidates(g.position.x, g.position.y, 
                          g.size + Config::FOOD_RADIUS + g.speed * 0.1f, // Add speed factor
                          foodCandidates);
                
                for (int idx : foodCandidates) {
                    if (idx < 0 || idx >= (int)foods.size()) continue;
                    Resource &food = foods[idx];
                    if (!food.active) continue;
                    
                    float dist = Vector2Distance(g.position, food.position);
                    if (dist < g.size + Config::FOOD_RADIUS) {
                        g.Feed(food.value);
                        food.Respawn(envW, envH);
                        ClearResourceClaim(idx, false); // Clear claim when consumed
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
                        ClearResourceClaim(idx, true); // Clear claim when consumed
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

bool Simulation::ShouldEndSimulation() const {
    return grubs.empty() || totalTime > 600.0f || userStopped;
}

void Simulation::EndSimulation() {
    simulationEnded = true;
}

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