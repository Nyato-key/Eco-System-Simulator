// UniformGrid.h
#pragma once
#include <vector>
#include "Config.h"

struct UniformGrid {
    int cols = 0, rows = 0;
    float cellSize = Config::GRID_CELL_SIZE;
    int width = 0, height = 0;
    std::vector<std::vector<int>> cells;

    void Init(int envWidth, int envHeight, float cs = Config::GRID_CELL_SIZE);
    void Clear();
    int CellIndexForPos(float x, float y) const;
    void Insert(int id, float x, float y);
    void CollectCandidates(float cx, float cy, float r, std::vector<int>& out) const;
    
    // Optional: Add bounds checking helper
    bool IsPositionValid(float x, float y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};