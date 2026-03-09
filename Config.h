// Config.h
#pragma once

// Global configuration and constants
namespace Config {
    // Simulation
    static constexpr float FIXED_STEP = 1.0f / 60.0f;
    static constexpr float REPRODUCE_INTERVAL = 0.5f;
    static constexpr float SAMPLE_INTERVAL = 1.0f;
    
    // Trait bounds

    static constexpr float SPEED_MIN = 1.0f;    // Increased from 0.2f
    static constexpr float SPEED_MAX = 200.0f;  // Increased from 20.0f
    static constexpr float SIGHT_MIN = 50.0f;       // Increased from 20.0f
    static constexpr float SIGHT_MAX = 800.0f;      // Increased from 400.0f
    static constexpr float METAB_MIN = 0.01f;
    static constexpr float METAB_MAX = 0.2f;
    static constexpr float MUTATION_CHANCE_MIN = 0.0f;
    static constexpr float MUTATION_CHANCE_MAX = 100.0f;
    
    // Reproduction
    static constexpr float BASE_REPRO_B = 0.5f;
    static constexpr float REPRO_COOLDOWN = 10.0f;
    static constexpr float MATING_RANGE_FACTOR = 1.0f;
    
    // Resources
    static constexpr float FOOD_RADIUS = 6.0f;
    static constexpr float WATER_RADIUS = 6.0f;
    
    // Performance
    static constexpr float GRID_CELL_SIZE = 80.0f;
    static constexpr int MAX_GRUBS = 5000;
    
    // Default values
    static constexpr int DEFAULT_GRUBS = 40;
    static constexpr int DEFAULT_FOOD = 90;
    static constexpr int DEFAULT_WATER = 60;
    static constexpr float DEFAULT_MUTATION_CHANCE = 5.0f;
    static constexpr int DEFAULT_ENV_WIDTH = 1200;
    static constexpr int DEFAULT_ENV_HEIGHT = 800;
    
    // Crowding calculation (area per grub)
    static constexpr float AREA_PER_GRUB = 2000.0f;
    
    // Cost multipliers for speed/sight
    static constexpr float HUNGER_COST_PER_SPEED = 0.001f;   // Extra hunger per speed unit
    static constexpr float THIRST_COST_PER_SIGHT = 0.0005f;  // Extra thirst per sight unit
}