// UniformGrid.cpp
#include "UniformGrid.h"
#include "Utils.h"
#include <set> // Optional: for duplicate prevention

void UniformGrid::Init(int envWidth, int envHeight, float cs) {
    width = envWidth; 
    height = envHeight;
    cellSize = cs;
    cols = std::max(1, int(std::ceil((float)width / cellSize)));
    rows = std::max(1, int(std::ceil((float)height / cellSize)));
    cells.assign(cols * rows, {});
}

void UniformGrid::Clear() {
    for (auto &c : cells) c.clear();
}

int UniformGrid::CellIndexForPos(float x, float y) const {
    int cx = clampVal(int(x / cellSize), 0, cols - 1);
    int cy = clampVal(int(y / cellSize), 0, rows - 1);
    return cy * cols + cx;
}

void UniformGrid::Insert(int id, float x, float y) {
    int idx = CellIndexForPos(x, y);
    cells[idx].push_back(id);
}

void UniformGrid::CollectCandidates(float cx, float cy, float r, std::vector<int>& out) const {
    int left = clampVal(int((cx - r) / cellSize), 0, cols - 1);
    int right = clampVal(int((cx + r) / cellSize), 0, cols - 1);
    int top = clampVal(int((cy - r) / cellSize), 0, rows - 1);
    int bottom = clampVal(int((cy + r) / cellSize), 0, rows - 1);
    
    // Optional: Reserve space for performance
    int estimatedCells = (right - left + 1) * (bottom - top + 1);
    int estimatedItems = estimatedCells * 10; // Rough estimate
    out.reserve(out.size() + estimatedItems);
    
    for (int cyi = top; cyi <= bottom; ++cyi) {
        for (int cxi = left; cxi <= right; ++cxi) {
            int idx = cyi * cols + cxi;
            const auto &cell = cells[idx];
            out.insert(out.end(), cell.begin(), cell.end());
        }
    }
    
    // Optional: Remove duplicates if objects can span multiple cells
    // This is more expensive but ensures uniqueness
    // std::set<int> unique(out.begin(), out.end());
    // out.assign(unique.begin(), unique.end());
}