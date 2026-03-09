// main.cpp
#include "raylib.h"
#include "Simulation.h"
#include "Utils.h"
#include "Config.h"
#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

#ifndef PI
    #define PI 3.14159265358979323846f
#endif

bool ShowConfigDialog(int& grubs, int& food, int& water, float& mutation, int& envWidth, int& envHeight) {
    const int screenWidth = 600;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Grub Simulation - Configuration");
    SetTargetFPS(60);
    
    grubs = Config::DEFAULT_GRUBS;
    food = Config::DEFAULT_FOOD;
    water = Config::DEFAULT_WATER;
    mutation = Config::DEFAULT_MUTATION_CHANCE;
    envWidth = Config::DEFAULT_ENV_WIDTH;
    envHeight = Config::DEFAULT_ENV_HEIGHT;
    
    bool confirmed = false;
    bool exitProgram = false;
    
    int crowdingFactor = static_cast<int>((envWidth * envHeight) / Config::AREA_PER_GRUB);
    
    float widthSlider = (float)(envWidth - 400) / (2000 - 400);
    float heightSlider = (float)(envHeight - 400) / (1500 - 400);
    float grubsSlider = (float)(grubs - 1) / (crowdingFactor - 1);
    float foodSlider = (float)(food - 1) / (300 - 1);
    float waterSlider = (float)(water - 1) / (300 - 1);
    float mutationSlider = mutation / 100.0f;
    
    Rectangle widthSliderRect = {250, 100, 150, 20};
    Rectangle heightSliderRect = {250, 140, 150, 20};
    Rectangle grubsSliderRect = {250, 180, 150, 20};
    Rectangle foodSliderRect = {250, 220, 150, 20};
    Rectangle waterSliderRect = {250, 260, 150, 20};
    Rectangle mutationSliderRect = {250, 300, 150, 20};
    
    Rectangle startButton = {200, 390, 200, 40};
    Rectangle exitButton = {200, 450, 200, 40};
    
    while (!WindowShouldClose() && !confirmed && !exitProgram) {
        envWidth = 400 + (int)(widthSlider * (2000 - 400));
        envHeight = 400 + (int)(heightSlider * (1500 - 400));
        
        crowdingFactor = static_cast<int>((envWidth * envHeight) / Config::AREA_PER_GRUB);
        grubs = 1 + (int)(grubsSlider * (crowdingFactor - 1));
        
        food = 1 + (int)(foodSlider * (300 - 1));
        water = 1 + (int)(waterSlider * (300 - 1));
        mutation = mutationSlider * 100.0f;
        
        if (grubs > crowdingFactor) {
            grubs = crowdingFactor;
            grubsSlider = (float)(grubs - 1) / (crowdingFactor - 1);
        }
        
        Vector2 mousePos = GetMousePosition();
        bool mousePressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        
        if (mousePressed) {
            if (CheckCollisionPointRec(mousePos, widthSliderRect)) {
                widthSlider = (mousePos.x - 250) / 150;
                widthSlider = clampVal(widthSlider, 0.0f, 1.0f);
            }
            if (CheckCollisionPointRec(mousePos, heightSliderRect)) {
                heightSlider = (mousePos.x - 250) / 150;
                heightSlider = clampVal(heightSlider, 0.0f, 1.0f);
            }
            if (CheckCollisionPointRec(mousePos, grubsSliderRect)) {
                grubsSlider = (mousePos.x - 250) / 150;
                grubsSlider = clampVal(grubsSlider, 0.0f, 1.0f);
            }
            if (CheckCollisionPointRec(mousePos, foodSliderRect)) {
                foodSlider = (mousePos.x - 250) / 150;
                foodSlider = clampVal(foodSlider, 0.0f, 1.0f);
            }
            if (CheckCollisionPointRec(mousePos, waterSliderRect)) {
                waterSlider = (mousePos.x - 250) / 150;
                waterSlider = clampVal(waterSlider, 0.0f, 1.0f);
            }
            if (CheckCollisionPointRec(mousePos, mutationSliderRect)) {
                mutationSlider = (mousePos.x - 250) / 150;
                mutationSlider = clampVal(mutationSlider, 0.0f, 1.0f);
            }
        }
        
        if (CheckCollisionPointRec(mousePos, startButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            confirmed = true;
        }
        
        if (CheckCollisionPointRec(mousePos, exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            exitProgram = true;
        }
        
        BeginDrawing();
        ClearBackground(Color{30, 30, 50, 255});
        
        DrawText("Grub Simulation Configuration", 20, 20, 28, LIGHTGRAY);
        DrawText("Set initial parameters:", 20, 60, 20, LIGHTGRAY);
        
        DrawText(TextFormat("Environment Width: %d", envWidth), 20, 100, 18, LIGHTGRAY);
        DrawRectangleRec(widthSliderRect, DARKGRAY);
        DrawRectangle(250 + (int)(widthSlider * 150) - 5, 100, 10, 20, LIGHTGRAY);
        
        DrawText(TextFormat("Environment Height: %d", envHeight), 20, 140, 18, LIGHTGRAY);
        DrawRectangleRec(heightSliderRect, DARKGRAY);
        DrawRectangle(250 + (int)(heightSlider * 150) - 5, 140, 10, 20, LIGHTGRAY);
        
        DrawText(TextFormat("Initial Grubs: %d/%d", grubs, crowdingFactor), 20, 180, 18, LIGHTGRAY);
        DrawRectangleRec(grubsSliderRect, DARKGRAY);
        DrawRectangle(250 + (int)(grubsSlider * 150) - 5, 180, 10, 20, LIGHTGRAY);
        
        DrawText(TextFormat("Initial Food: %d", food), 20, 220, 18, LIGHTGRAY);
        DrawRectangleRec(foodSliderRect, DARKGRAY);
        DrawRectangle(250 + (int)(foodSlider * 150) - 5, 220, 10, 20, LIGHTGRAY);
        
        DrawText(TextFormat("Initial Water: %d", water), 20, 260, 18, LIGHTGRAY);
        DrawRectangleRec(waterSliderRect, DARKGRAY);
        DrawRectangle(250 + (int)(waterSlider * 150) - 5, 260, 10, 20, LIGHTGRAY);
        
        DrawText(TextFormat("Mutation Rate: %.1f%%", mutation), 20, 300, 18, LIGHTGRAY);
        DrawRectangleRec(mutationSliderRect, DARKGRAY);
        DrawRectangle(250 + (int)(mutationSlider * 150) - 5, 300, 10, 20, LIGHTGRAY);
        
        DrawText(TextFormat("Auto-Calculated Maximum Grubs: %d", crowdingFactor), 20, 330, 16, YELLOW);
        DrawText("Based on environment size and crowding rules", 20, 350, 12, GRAY);
        
        DrawRectangleRec(startButton, GREEN);
        DrawText("Start Simulation", startButton.x + 20, startButton.y + 10, 20, BLACK);
        
        DrawRectangleRec(exitButton, RED);
        DrawText("Exit", exitButton.x + 80, exitButton.y + 10, 20, BLACK);
        
        DrawText("Click and drag sliders to adjust parameters", 20, 520, 14, GRAY);
        
        EndDrawing();
    }
    
    bool result = confirmed;
    CloseWindow();
    
    if (exitProgram) return false;
    return result;
}

// Graph data structure
struct GraphData {
    std::vector<float> times;
    std::vector<int> populations;
    std::vector<float> avgSpeeds;
    std::vector<float> avgSights;
    std::vector<float> avgMutations;
};

// Function to load data from CSV
GraphData LoadGraphDataFromCSV(const std::string& filename) {
    GraphData data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cout << "ERROR: Could not open data file: " << filename << std::endl;
        return data;
    }
    
    std::string line;
    bool firstLine = true;
    
    while (std::getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue;
        }
        
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 6) {
            try {
                data.times.push_back(std::stof(tokens[0]));
                data.populations.push_back(std::stoi(tokens[1]));
                data.avgSpeeds.push_back(std::stof(tokens[2]));
                data.avgSights.push_back(std::stof(tokens[3]));
                data.avgMutations.push_back(std::stof(tokens[5]));
            } catch (...) {
                std::cout << "Warning: Could not parse line: " << line << std::endl;
            }
        }
    }
    
    file.close();
    std::cout << "Loaded " << data.times.size() << " data points from CSV" << std::endl;
    return data;
}

// SINGLE DrawGraph function
void DrawGraph(Rectangle area, const char* title, 
               const std::vector<float>& xData, const std::vector<float>& yData,
               Color color, const char* xLabel, const char* yLabel,
               bool showPoints = true) {
    
    if (xData.empty() || yData.empty()) return;
    
    float maxX = *std::max_element(xData.begin(), xData.end());
    float minY = *std::min_element(yData.begin(), yData.end());
    float maxY = *std::max_element(yData.begin(), yData.end());
    
    float yRange = maxY - minY;
    if (yRange < 0.01f) yRange = 1.0f;
    minY = minY - yRange * 0.1f;
    maxY = maxY + yRange * 0.1f;
    
    // Draw graph background
    DrawRectangleRec(area, Color{20, 20, 30, 255});
    DrawRectangleLinesEx(area, 2, Color{60, 60, 70, 255});
    
    // Draw title (centered)
    int titleWidth = MeasureText(title, 16);
    DrawText(title, area.x + (area.width - titleWidth)/2, area.y + 8, 16, WHITE);
    
    // Calculate inner area
    float leftMargin = 60.0f;
    float rightMargin = 20.0f;
    float topMargin = 35.0f;
    float bottomMargin = 40.0f;
    
    Rectangle innerArea = {
        area.x + leftMargin,
        area.y + topMargin,
        area.width - leftMargin - rightMargin,
        area.height - topMargin - bottomMargin
    };
    
    // Draw axes
    // X-axis
    DrawLineEx(
        Vector2{innerArea.x, innerArea.y + innerArea.height},
        Vector2{innerArea.x + innerArea.width, innerArea.y + innerArea.height},
        2.0f, LIGHTGRAY
    );
    
    // Y-axis
    DrawLineEx(
        Vector2{innerArea.x, innerArea.y},
        Vector2{innerArea.x, innerArea.y + innerArea.height},
        2.0f, LIGHTGRAY
    );
    
    // Draw axis labels
    // X-axis label
    int xLabelWidth = MeasureText(xLabel, 14);
    DrawText(xLabel, area.x + (area.width - xLabelWidth)/2, 
             area.y + area.height - 20, 14, LIGHTGRAY);
    
    // Y-axis label (vertical)
    // Draw vertically by drawing character by character
    for (int i = 0; yLabel[i] != '\0'; i++) {
        char ch[2] = {yLabel[i], '\0'};
        DrawText(ch, area.x + 15, area.y + topMargin + 100 + i * 20, 14, LIGHTGRAY);
    }
    
    // Draw Y-axis scale labels
    for (int i = 0; i <= 3; i++) {
        float value = minY + (maxY - minY) * (float)i / 3.0f;
        char label[20];
        snprintf(label, 20, "%.0f", value);
        
        float yPos = innerArea.y + innerArea.height - (innerArea.height * i / 3.0f);
        
        // Right-align
        int textWidth = MeasureText(label, 11);
        DrawText(label, innerArea.x - textWidth - 5, yPos - 6, 11, GRAY);
        
        // Draw light grid line
        if (i > 0 && i < 3) {
            DrawLine(innerArea.x, yPos, innerArea.x + innerArea.width, yPos, 
                    Color{40, 40, 50, 255});
        }
    }
    
    // Draw X-axis scale labels
    int numXLabels = 5;
    for (int i = 0; i <= numXLabels; i++) {
        float value = maxX * (float)i / (float)numXLabels;
        char label[20];
        
        if (maxX < 100) {
            snprintf(label, 20, "%.0f", value);
        } else {
            snprintf(label, 20, "%.0f", value);
        }
        
        float xPos = innerArea.x + (innerArea.width * i / (float)numXLabels);
        int textWidth = MeasureText(label, 11);
        
        // Center text under the tick
        DrawText(label, xPos - textWidth/2, innerArea.y + innerArea.height + 5, 11, GRAY);
        
        // Draw tick marks
        DrawLine(xPos, innerArea.y + innerArea.height - 3, 
                xPos, innerArea.y + innerArea.height + 3, LIGHTGRAY);
    }
    
    // Draw data line
    for (size_t i = 0; i < xData.size() - 1; i++) {
        float xPos1 = innerArea.x + (xData[i] / maxX) * innerArea.width;
        float yPos1 = innerArea.y + innerArea.height - 
                     ((yData[i] - minY) / (maxY - minY)) * innerArea.height;
        
        float xPos2 = innerArea.x + (xData[i+1] / maxX) * innerArea.width;
        float yPos2 = innerArea.y + innerArea.height - 
                     ((yData[i+1] - minY) / (maxY - minY)) * innerArea.height;
        
        // Draw line connecting points
        DrawLineEx(Vector2{xPos1, yPos1}, Vector2{xPos2, yPos2}, 2.5f, color);
        
        // Draw points only for sparse data
        if (showPoints && xData.size() < 50) {
            DrawCircle(xPos1, yPos1, 3, color);
        }
    }
}

void ShowGraphs(const GraphData& data) {
    const int screenWidth = 1600;
    const int screenHeight = 1000;
    
    InitWindow(screenWidth, screenHeight, "Grub Simulation - Results Analysis");
    SetTargetFPS(60);
    
    // Header area
    int headerHeight = 90;
    int graphAreaStartY = headerHeight;
    
    Rectangle graphAreas[4];
    int margin = 30;
    int statsWidth = 300;
    int graphsWidth = screenWidth - statsWidth - 2 * margin;
    int graphWidth = graphsWidth / 2 - margin;
    int graphHeight = (screenHeight - graphAreaStartY - 3 * margin) / 2;
    
    graphAreas[0] = { (float)margin, (float)(graphAreaStartY + margin), (float)graphWidth, (float)graphHeight };  // Top-left
    graphAreas[1] = { (float)(margin * 2 + graphWidth), (float)(graphAreaStartY + margin), (float)graphWidth, (float)graphHeight };  // Top-right
    graphAreas[2] = { (float)margin, (float)(graphAreaStartY + margin * 2 + graphHeight), (float)graphWidth, (float)graphHeight };  // Bottom-left
    graphAreas[3] = { (float)(margin * 2 + graphWidth), (float)(graphAreaStartY + margin * 2 + graphHeight), (float)graphWidth, (float)graphHeight };  // Bottom-right
    
    // Statistics panel on the right (below header)
    Rectangle statsPanel = {
        (float)(screenWidth - statsWidth - margin),
        (float)(graphAreaStartY + margin),
        (float)statsWidth,
        (float)(screenHeight - graphAreaStartY - 2 * margin)
    };
    
    // Convert population to float for graphing
    std::vector<float> populationsFloat;
    for (int pop : data.populations) {
        populationsFloat.push_back((float)pop);
    }
    
    // Variables for hover tooltip
    int hoveredIndex = -1;
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{15, 15, 25, 255});
        
        // Draw header area with background to prevent text overlap
        DrawRectangle(0, 0, screenWidth, headerHeight, Color{20, 20, 35, 255});
        DrawLine(0, headerHeight, screenWidth, headerHeight, Color{60, 60, 70, 255});
        
        // Draw header text - properly centered and spaced
        DrawText("Grub Simulation - Results Analysis", 
                screenWidth/2 - MeasureText("Grub Simulation - Results Analysis", 32)/2, 
                20, 32, LIGHTGRAY);
        
        if (!data.times.empty()) {
            DrawText(TextFormat("Total Samples: %d | Duration: %.1fs", 
                      (int)data.times.size(), data.times.back()), 
                      screenWidth/2 - MeasureText(TextFormat("Total Samples: %d | Duration: %.1fs", 
                      (int)data.times.size(), data.times.back()), 16)/2, 
                      65, 16, GRAY);
        }
        
        // Draw all 4 graphs
        if (!data.times.empty()) {
            // Reset hover state
            hoveredIndex = -1;
            
            // 1. Population over time
            DrawGraph(graphAreas[0], "Population Over Time", 
                     data.times, populationsFloat, RED,
                     "Time (seconds)", "Population");
            
            // 2. Speed over time
            DrawGraph(graphAreas[1], "Average Speed Evolution", 
                     data.times, data.avgSpeeds, Color{0, 200, 0, 255},
                     "Time (seconds)", "Speed");
            
            // 3. Sight over time
            DrawGraph(graphAreas[2], "Average Sight Evolution", 
                     data.times, data.avgSights, SKYBLUE,
                     "Time (seconds)", "Sight");
            
            // 4. Mutation rate over time
            DrawGraph(graphAreas[3], "Average Mutation Rate", 
                     data.times, data.avgMutations, Color{200, 100, 255, 255},
                     "Time (seconds)", "Mutation %");
            
            // Check mouse hover on graphs for tooltip
            Vector2 mousePos = GetMousePosition();
            
            // Check which graph is being hovered
            for (int i = 0; i < 4; i++) {
                if (CheckCollisionPointRec(mousePos, graphAreas[i])) {
                    // Find nearest data point
                    float graphInnerX = graphAreas[i].x + 60.0f;  // Left margin
                    float graphInnerWidth = graphAreas[i].width - 60.0f - 20.0f;  // minus margins
                    
                    // Calculate which data point is nearest to mouse
                    float mouseRelX = (mousePos.x - graphInnerX) / graphInnerWidth;
                    mouseRelX = clampVal(mouseRelX, 0.0f, 1.0f);  
                    float targetTime = mouseRelX * data.times.back();
                    
                    // Find nearest time point
                    float minDist = 999999.0f;
                    for (size_t j = 0; j < data.times.size(); j++) {
                        float dist = fabs(data.times[j] - targetTime);
                        if (dist < minDist) {
                            minDist = dist;
                            hoveredIndex = j;
                        }
                    }
                    break;
                }
            }
            
            // Draw statistics panel (right side)
            DrawRectangleRec(statsPanel, Color{0, 0, 0, 180});
            DrawRectangleLinesEx(statsPanel, 2, Color{60, 60, 70, 255});
            
            // Statistics title
            DrawText("Statistics", statsPanel.x + 15, statsPanel.y + 15, 22, WHITE);
            
            // Key statistics
            int yPos = statsPanel.y + 50;
            
            // Final values
            if (!data.times.empty()) {
                DrawText("Final Values:", statsPanel.x + 15, yPos, 18, YELLOW);
                yPos += 30;
                
                DrawText(TextFormat("Population: %d", data.populations.back()), 
                        statsPanel.x + 15, yPos, 16, LIGHTGRAY);
                yPos += 25;
                
                DrawText(TextFormat("Speed: %.1f", data.avgSpeeds.back()), 
                        statsPanel.x + 15, yPos, 16, Color{0, 200, 0, 255});
                yPos += 25;
                
                DrawText(TextFormat("Sight: %.1f", data.avgSights.back()), 
                        statsPanel.x + 15, yPos, 16, SKYBLUE);
                yPos += 25;
                
                DrawText(TextFormat("Mutation: %.1f%%", data.avgMutations.back()), 
                        statsPanel.x + 15, yPos, 16, Color{200, 100, 255, 255});
                yPos += 40;
            }
            
            // Range statistics
            float maxPopulation = 0;
            float minPopulation = 999999;
            for (int pop : data.populations) {
                if (pop > maxPopulation) maxPopulation = pop;
                if (pop < minPopulation) minPopulation = pop;
            }
            
            DrawText("Range Statistics:", statsPanel.x + 15, yPos, 18, YELLOW);
            yPos += 30;
            
            DrawText(TextFormat("Max Population: %d", (int)maxPopulation), 
                    statsPanel.x + 15, yPos, 16, LIGHTGRAY);
            yPos += 25;
            
            DrawText(TextFormat("Min Population: %d", (int)minPopulation), 
                    statsPanel.x + 15, yPos, 16, LIGHTGRAY);
            yPos += 25;
            
            // Calculate average values
            float avgSpeed = 0, avgSight = 0, avgMutation = 0;
            int avgCount = 0;
            
            for (size_t i = 0; i < data.times.size(); i++) {
                avgSpeed += data.avgSpeeds[i];
                avgSight += data.avgSights[i];
                avgMutation += data.avgMutations[i];
                avgCount++;
            }
            
            if (avgCount > 0) {
                avgSpeed /= avgCount;
                avgSight /= avgCount;
                avgMutation /= avgCount;
                
                DrawText(TextFormat("Avg Speed: %.1f", avgSpeed), 
                        statsPanel.x + 15, yPos, 16, LIGHTGRAY);
                yPos += 25;
                
                DrawText(TextFormat("Avg Sight: %.1f", avgSight), 
                        statsPanel.x + 15, yPos, 16, LIGHTGRAY);
                yPos += 25;
            }
            
            // Legend at bottom
            yPos = statsPanel.y + statsPanel.height - 140;
            DrawText("Legend:", statsPanel.x + 15, yPos, 18, WHITE);
            yPos += 25;
            
            DrawRectangle(statsPanel.x + 15, yPos, 12, 12, RED);
            DrawText("Population", statsPanel.x + 35, yPos - 2, 16, RED);
            yPos += 22;
            
            DrawRectangle(statsPanel.x + 15, yPos, 12, 12, Color{0, 200, 0, 255});
            DrawText("Speed", statsPanel.x + 35, yPos - 2, 16, Color{0, 200, 0, 255});
            yPos += 22;
            
            DrawRectangle(statsPanel.x + 15, yPos, 12, 12, SKYBLUE);
            DrawText("Sight", statsPanel.x + 35, yPos - 2, 16, SKYBLUE);
            yPos += 22;
            
            DrawRectangle(statsPanel.x + 15, yPos, 12, 12, Color{200, 100, 255, 255});
            DrawText("Mutation", statsPanel.x + 35, yPos - 2, 16, Color{200, 100, 255, 255});
            
            // Draw hover tooltip if hovering over a graph
            if (hoveredIndex >= 0 && hoveredIndex < (int)data.times.size()) {
                // Draw tooltip near mouse
                float tooltipX = mousePos.x + 20;
                float tooltipY = mousePos.y + 20;
                float tooltipWidth = 250;
                float tooltipHeight = 120;
                
                // Make sure tooltip stays on screen
                if (tooltipX + tooltipWidth > screenWidth) tooltipX = mousePos.x - tooltipWidth - 10;
                if (tooltipY + tooltipHeight > screenHeight) tooltipY = mousePos.y - tooltipHeight - 10;
                
                DrawRectangle(tooltipX, tooltipY, tooltipWidth, tooltipHeight, Color{0, 0, 0, 230});
                DrawRectangleLinesEx(Rectangle{tooltipX, tooltipY, tooltipWidth, tooltipHeight}, 2, WHITE);
                
                DrawText("Data Point Details:", tooltipX + 10, tooltipY + 10, 16, YELLOW);
                DrawText(TextFormat("Time: %.1fs", data.times[hoveredIndex]), tooltipX + 10, tooltipY + 35, 14, WHITE);
                DrawText(TextFormat("Population: %d", data.populations[hoveredIndex]), tooltipX + 10, tooltipY + 55, 14, RED);
                DrawText(TextFormat("Speed: %.1f", data.avgSpeeds[hoveredIndex]), tooltipX + 10, tooltipY + 75, 14, Color{0, 200, 0, 255});
                DrawText(TextFormat("Sight: %.1f", data.avgSights[hoveredIndex]), tooltipX + 10, tooltipY + 95, 14, SKYBLUE);
            }
            
        } else {
            // No data message
            DrawText("No simulation data available to display.", 
                     screenWidth/2 - MeasureText("No simulation data available to display.", 24)/2, 
                     screenHeight/2, 24, RED);
            DrawText("Run a simulation first and save data to CSV.", 
                     screenWidth/2 - MeasureText("Run a simulation first and save data to CSV.", 18)/2, 
                     screenHeight/2 + 40, 18, GRAY);
        }
        
        // Instructions at bottom
        DrawText("Move mouse over graphs to see data points | Press ESC to exit", 
                screenWidth/2 - MeasureText("Move mouse over graphs to see data points | Press ESC to exit", 16)/2, 
                screenHeight - 30, 16, GRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
}


int main() {
    initRNG();
    
    // Get configuration from user
    int initialGrubs, initialFood, initialWater, envWidth, envHeight;
    float mutationChance;
    
    if (!ShowConfigDialog(initialGrubs, initialFood, initialWater, mutationChance, 
                         envWidth, envHeight)) {
        return 0;
    }
    
    // Initialize main simulation window
    InitWindow(envWidth, envHeight, "Grub Simulation - Press ESC to stop and view graphs");
    SetTargetFPS(60);
    
    Simulation sim;
    sim.initialGrubs = initialGrubs;
    sim.initialFood = initialFood;
    sim.initialWater = initialWater;
    sim.baseMutationChance = mutationChance;
    sim.Init(envWidth, envHeight);
    
    float accumulator = 0.0f;
    
    // Calculate the ACTUAL maximum grubs for this environment
    int currentMaxGrubs = static_cast<int>((envWidth * envHeight) / Config::AREA_PER_GRUB);
    
    // Main game loop
    while (!WindowShouldClose() && !sim.simulationEnded) {
        // Input handling
        if (IsKeyPressed(KEY_SPACE)) sim.paused = !sim.paused;
        if (IsKeyPressed(KEY_R)) { sim.Init(envWidth, envHeight); accumulator = 0.0f; }
        if (IsKeyPressed(KEY_D)) sim.debugDraw = !sim.debugDraw;
        if (IsKeyPressed(KEY_EQUAL)) sim.timeScale = clampVal(sim.timeScale + 0.25f, 0.1f, 8.0f);
        if (IsKeyPressed(KEY_MINUS)) sim.timeScale = clampVal(sim.timeScale - 0.25f, 0.1f, 8.0f);
        if (IsKeyPressed(KEY_S) && sim.paused) sim.UpdateFixedStep();
        
        // Stop simulation when ESC is pressed
        if (IsKeyPressed(KEY_ESCAPE)) {
            sim.StopSimulation();
        }
        
        // Fixed timestep update
        float realDt = GetFrameTime();
        if (!sim.paused) {
            accumulator += realDt * sim.timeScale;
            accumulator = clampVal(accumulator, 0.0f, 0.5f);
            while (accumulator >= Config::FIXED_STEP) {
                sim.UpdateFixedStep();
                accumulator -= Config::FIXED_STEP;
            }
        }
        
        // Rendering
        BeginDrawing();
        ClearBackground(Color{10, 10, 30, 255});
        
        sim.Draw();
        
        // UI Overlay
        DrawRectangle(8, 8, 380, 160, Color{0,0,0,180});
        DrawText(TextFormat("Grubs: %d/%d", (int)sim.grubs.size(), currentMaxGrubs), 16, 12, 20, LIME);
        DrawText(TextFormat("Food: %d  Water: %d", (int)sim.foods.size(), (int)sim.waters.size()), 16, 36, 18, YELLOW);
        DrawText(TextFormat("TimeScale: %.2fx (+/-)", sim.timeScale), 16, 58, 18, LIGHTGRAY);
        DrawText(TextFormat("Paused (SPACE): %s", sim.paused ? "YES" : "NO"), 16, 80, 18, LIGHTGRAY);
        DrawText(TextFormat("Time: %.1fs", sim.totalTime), 16, 102, 18, LIGHTGRAY);
        
        // Show calculated crowding factor
        DrawText(TextFormat("Crowding Factor: %d", currentMaxGrubs), 16, 124, 16, ORANGE);
        
        DrawText("ESC=Stop&Save | Space=Pause | R=Restart | +/-=Speed", 16, 144, 12, GRAY);
        
        EndDrawing();
    }
    
    // Save data before closing
    sim.SaveDataToFile("simulation_data.csv");
    
    CloseWindow();

    // LOAD AND DISPLAY GRAPHS
    GraphData graphData = LoadGraphDataFromCSV("simulation_data.csv");
    if (!graphData.times.empty()) {
        ShowGraphs(graphData);
    } else {
        std::cout << "No simulation data to display." << std::endl;
    }
    
    return 0;
}