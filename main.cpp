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


// Configuration dialog UI
bool ShowConfigDialog(int& outGrubs, int& outFood, int& outWater, float& outMutation, int& outEnvWidth, int& outEnvHeight) {
    const int screenWidth = 600;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Grub Simulation - Configuration");
    SetTargetFPS(60);

    // Defaults from Config
    int grubs = Config::DEFAULT_GRUBS;
    int food = Config::DEFAULT_FOOD;
    int water = Config::DEFAULT_WATER;
    float mutation = Config::DEFAULT_MUTATION_CHANCE;
    int envWidth = Config::DEFAULT_ENV_WIDTH;
    int envHeight = Config::DEFAULT_ENV_HEIGHT;

    bool confirmed = false;
    bool exitProgram = false;

    // Slider domain constants
    const float widthMin = 400.0f, widthMax = 2000.0f;
    const float heightMin = 400.0f, heightMax = 1500.0f;
    const float foodMax = 300.0f, waterMax = 300.0f;

    auto computeCrowding = [&](int w, int h) {
        return static_cast<int>((float(w) * float(h)) / Config::AREA_PER_GRUB);
    };

    int crowdingFactor = computeCrowding(envWidth, envHeight);

    // Normalized slider values in [0,1]
    float widthSlider = (envWidth - (int)widthMin) / (widthMax - widthMin);
    float heightSlider = (envHeight - (int)heightMin) / (heightMax - heightMin);
    float grubsSlider = (grubs - 1) / float(std::max(1, crowdingFactor - 1));
    float foodSlider = (food - 1) / (foodMax - 1.0f);
    float waterSlider = (water - 1) / (waterMax - 1.0f);
    float mutationSlider = mutation / 100.0f;

    // Slider geometry - Shape var = {x, y, w, h}
    Rectangle widthSliderRect = {250, 100, 150, 20};
    Rectangle heightSliderRect = {250, 140, 150, 20};
    Rectangle grubsSliderRect = {250, 180, 150, 20};
    Rectangle foodSliderRect = {250, 220, 150, 20};
    Rectangle waterSliderRect = {250, 260, 150, 20};
    Rectangle mutationSliderRect = {250, 300, 150, 20};

    Rectangle startButton = {200, 390, 200, 40};
    Rectangle exitButton  = {200, 450, 200, 40};

    // Update slider by rectangle and normalized value
    auto updateSliderIfPressed = [&](const Rectangle &r, float &s, const Vector2 &mouse, bool pressed) {
        if (pressed && CheckCollisionPointRec(mouse, r)) {
            s = (mouse.x - r.x) / r.width;
            s = clampVal(s, 0.0f, 1.0f);
        }
    };

    // helper: draw a slider thumb given normalized position
    auto drawSliderThumb = [&](const Rectangle &r, float s) {
        DrawRectangleRec(r, DARKGRAY);
        DrawRectangle((int)(r.x + s * r.width) - 5, (int)r.y, 10, (int)r.height, LIGHTGRAY);
    };

    // Main UI loop
    while (!WindowShouldClose() && !confirmed && !exitProgram) {
        // Update numerical values from sliders
        envWidth = (int)(widthMin + widthSlider * (widthMax - widthMin));
        envHeight = (int)(heightMin + heightSlider * (heightMax - heightMin));
        crowdingFactor = computeCrowding(envWidth, envHeight);
        grubs = 1 + (int)(grubsSlider * std::max(1, crowdingFactor - 1));
        food = 1 + (int)(foodSlider * (foodMax - 1.0f));
        water = 1 + (int)(waterSlider * (waterMax - 1.0f));
        mutation = mutationSlider * 100.0f;

        if (grubs > crowdingFactor) {
            grubs = crowdingFactor;
            grubsSlider = (grubs - 1) / float(std::max(1, crowdingFactor - 1));
        }

        Vector2 mousePos = GetMousePosition();
        bool mousePressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        // Consolidated slider update
        updateSliderIfPressed(widthSliderRect, widthSlider, mousePos, mousePressed);
        updateSliderIfPressed(heightSliderRect, heightSlider, mousePos, mousePressed);
        updateSliderIfPressed(grubsSliderRect, grubsSlider, mousePos, mousePressed);
        updateSliderIfPressed(foodSliderRect, foodSlider, mousePos, mousePressed);
        updateSliderIfPressed(waterSliderRect, waterSlider, mousePos, mousePressed);
        updateSliderIfPressed(mutationSliderRect, mutationSlider, mousePos, mousePressed);

        if (CheckCollisionPointRec(mousePos, startButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            confirmed = true;
        }
        if (CheckCollisionPointRec(mousePos, exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            exitProgram = true;
        }

        // Draw UI
        BeginDrawing();
        ClearBackground(Color{30, 30, 50, 255});

        DrawText("Grub Simulation Configuration", 20, 20, 28, LIGHTGRAY);
        DrawText("Set initial parameters:", 20, 60, 20, LIGHTGRAY);

        DrawText(TextFormat("Environment Width: %d", envWidth), 20, 100, 18, LIGHTGRAY);
        drawSliderThumb(widthSliderRect, widthSlider);

        DrawText(TextFormat("Environment Height: %d", envHeight), 20, 140, 18, LIGHTGRAY);
        drawSliderThumb(heightSliderRect, heightSlider);

        DrawText(TextFormat("Initial Grubs: %d/%d", grubs, crowdingFactor), 20, 180, 18, LIGHTGRAY);
        drawSliderThumb(grubsSliderRect, grubsSlider);

        DrawText(TextFormat("Initial Food: %d", food), 20, 220, 18, LIGHTGRAY);
        drawSliderThumb(foodSliderRect, foodSlider);

        DrawText(TextFormat("Initial Water: %d", water), 20, 260, 18, LIGHTGRAY);
        drawSliderThumb(waterSliderRect, waterSlider);

        DrawText(TextFormat("Mutation Rate: %.1f%%", mutation), 20, 300, 18, LIGHTGRAY);
        drawSliderThumb(mutationSliderRect, mutationSlider);

        DrawText(TextFormat("Auto-Calculated Maximum Grubs: %d", crowdingFactor), 20, 330, 16, YELLOW);
        DrawText("Based on environment size and crowding rules", 20, 350, 12, GRAY);

        DrawRectangleRec(startButton, GREEN);
        DrawText("Start Simulation", startButton.x + 20, startButton.y + 10, 20, BLACK);

        DrawRectangleRec(exitButton, RED);
        DrawText("Exit", exitButton.x + 80, exitButton.y + 10, 20, BLACK);

        DrawText("Click and drag sliders to adjust parameters", 20, 520, 14, GRAY);

        EndDrawing();
    }

    CloseWindow();

    if (exitProgram) return false;

    // Return chosen values by reference
    outGrubs = grubs;
    outFood = food;
    outWater = water;
    outMutation = mutation;
    outEnvWidth = envWidth;
    outEnvHeight = envHeight;
    return true;
}

// Graph data structure & CSV loader
struct GraphData {
    std::vector<float> times;
    std::vector<int>   populations;
    std::vector<float> avgSpeeds;
    std::vector<float> avgSights;
    std::vector<float> avgMutations;
};

// Time(seconds),Population,AverageSpeed,AverageSight,AverageMetabolism,AverageMutationRate(%),CompetitionFactor,PopulationFitness
GraphData LoadGraphDataFromCSV(const std::string& filename) {
    GraphData data;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not open data file: " << filename << std::endl;
        return data;
    }

    std::string line;
    bool firstLine = true;

    // Reserve some space to reduce reallocations for medium-length runs
    data.times.reserve(512); //faster
    data.populations.reserve(512);
    data.avgSpeeds.reserve(512);
    data.avgSights.reserve(512);
    data.avgMutations.reserve(512);

    while (std::getline(file, line)) {
        if (firstLine) { firstLine = false; continue; } // skip header

        // Tokenize up to 6 columns; avoid creating dynamic vector each line
        std::stringstream ss(line);
        std::string tok;
        std::string toks[6];
        int idx = 0;
        while (idx < 6 && std::getline(ss, tok, ',')) {
            toks[idx++] = tok;
        }

        if (idx >= 6) {
            try {
                data.times.push_back(std::stof(toks[0])); //turns tokens from string to float
                data.populations.push_back(std::stoi(toks[1])); // to int
                data.avgSpeeds.push_back(std::stof(toks[2]));
                data.avgSights.push_back(std::stof(toks[3]));
                data.avgMutations.push_back(std::stof(toks[5]));
            } catch (...) {
                std::cout << "Warning: Could not parse CSV line: " << line << std::endl;
            }
        } // else: malformed line, skip
    }

    file.close();
    std::cout << "Loaded " << data.times.size() << " data points from CSV" << std::endl;
    return data;
}

// Graph drawing utility
// Single reusable DrawGraph function for different series
void DrawGraph(Rectangle area, const char* title,
               const std::vector<float>& xData, const std::vector<float>& yData,
               Color color, const char* xLabel, const char* yLabel,
               bool showPoints = true) {

    if (xData.empty() || yData.empty()) return;
    size_t n = std::min(xData.size(), yData.size());
    if (n < 2) return;

    // Compute ranges with guards
    float xMin = *std::min_element(xData.begin(), xData.begin() + n);
    float xMax = *std::max_element(xData.begin(), xData.begin() + n);
    float xRange = xMax - xMin;
    if (fabsf(xRange) < 1e-6f) xRange = 1.0f;

    float yMin = *std::min_element(yData.begin(), yData.begin() + n);
    float yMax = *std::max_element(yData.begin(), yData.begin() + n);
    float yRange = yMax - yMin;
    if (fabsf(yRange) < 1e-6f) yRange = 1.0f;

    // Add small padding so points don't touch frame
    yMin -= yRange * 0.1f;
    yMax += yRange * 0.1f;
    yRange = yMax - yMin;

    // Draw background and frame
    DrawRectangleRec(area, Color{20, 20, 30, 255});
    DrawRectangleLinesEx(area, 2, Color{60, 60, 70, 255});

    // Title
    int titleW = MeasureText(title, 16);
    DrawText(title, (int)(area.x + (area.width - titleW) / 2), (int)(area.y + 8), 16, WHITE);

    // Inner plot area (margins)
    const float leftMargin = 60.0f;
    const float rightMargin = 20.0f;
    const float topMargin = 35.0f;
    const float bottomMargin = 40.0f;

    Rectangle inner = {
        area.x + leftMargin,
        area.y + topMargin,
        area.width - leftMargin - rightMargin,
        area.height - topMargin - bottomMargin
    };

    // Axes
    DrawLineEx({inner.x, inner.y + inner.height}, {inner.x + inner.width, inner.y + inner.height}, 2.0f, LIGHTGRAY);
    DrawLineEx({inner.x, inner.y}, {inner.x, inner.y + inner.height}, 2.0f, LIGHTGRAY);

    // X label centered
    int xLabelW = MeasureText(xLabel, 14);
    DrawText(xLabel, (int)(area.x + (area.width - xLabelW) / 2), (int)(area.y + area.height - 20), 14, LIGHTGRAY);

    // Y label vertical (draw char by char)
    int yLabelX = (int)area.x + 20;
    int yLabelY = (int)(area.y + area.height / 2 - 50);  // Center vertically
    for (int i = 0; yLabel[i] != '\0'; ++i) {
        char ch[2] = { yLabel[i], '\0' };
        DrawText(ch, yLabelX, yLabelY + i * 15, 14, LIGHTGRAY);
    }
    // Y ticks (3 ticks)
    for (int i = 0; i <= 3; ++i) {
        float value = yMin + (yMax - yMin) * i / 3.0f;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", value);
        float yPos = inner.y + inner.height - (inner.height * i / 3.0f);
        int tw = MeasureText(buf, 11);
        DrawText(buf, (int)(inner.x - tw - 5), (int)(yPos - 6), 11, GRAY);
        if (i > 0 && i < 3) DrawLine(inner.x, yPos, inner.x + inner.width, yPos, Color{40,40,50,255});
    }

    // X ticks (up to 5)
    const int numXLabels = 5;
    for (int i = 0; i <= numXLabels; ++i) {
        float value = xMin + (xMax - xMin) * i / (float)numXLabels;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", value);
        float xPos = inner.x + (inner.width * i / (float)numXLabels);
        int tw = MeasureText(buf, 11);
        DrawText(buf, (int)(xPos - tw / 2), (int)(inner.y + inner.height + 5), 11, GRAY);
        DrawLine(xPos, inner.y + inner.height - 3, xPos, inner.y + inner.height + 3, LIGHTGRAY);
    }

    // If data is huge, downsample for drawing (simple decimation)
    size_t step = 1;
    if (n > 2000) step = (n / 1000) + 1;

    bool drawPoints = showPoints && n < 50;
    for (size_t i = 0; i + step < n; i += step) {
        size_t j = i + step;
        float x1 = inner.x + ((xData[i] - xMin) / xRange) * inner.width;
        float y1 = inner.y + inner.height - ((yData[i] - yMin) / yRange) * inner.height;
        float x2 = inner.x + ((xData[j] - xMin) / xRange) * inner.width;
        float y2 = inner.y + inner.height - ((yData[j] - yMin) / yRange) * inner.height;
        DrawLineEx({x1, y1}, {x2, y2}, 2.5f, color);
        if (drawPoints) DrawCircle((int)x1, (int)y1, 3, color);
    }
    if (drawPoints) { // draw last point
        float xLast = inner.x + ((xData[n-1] - xMin) / xRange) * inner.width;
        float yLast = inner.y + inner.height - ((yData[n-1] - yMin) / yRange) * inner.height;
        DrawCircle((int)xLast, (int)yLast, 3, color);
    }
}

// Results viewer (ShowGraphs)
// Uses DrawGraph to render 4 charts and a statistics panel
void ShowGraphs(const GraphData& data) {
    const int screenWidth = 1600;
    const int screenHeight = 1000;
    InitWindow(screenWidth, screenHeight, "Grub Simulation - Results Analysis");
    SetTargetFPS(60);

    const int headerHeight = 90;
    const int margin = 30;
    const int statsWidth = 300;
    int graphsWidth = screenWidth - statsWidth - 2 * margin;
    int graphWidth = graphsWidth / 2 - margin;
    int graphHeight = (screenHeight - headerHeight - 3 * margin) / 2;

    Rectangle graphAreas[4] = {
        { (float)margin, (float)(headerHeight + margin), (float)graphWidth, (float)graphHeight },
        { (float)(margin * 2 + graphWidth), (float)(headerHeight + margin), (float)graphWidth, (float)graphHeight },
        { (float)margin, (float)(headerHeight + margin * 2 + graphHeight), (float)graphWidth, (float)graphHeight },
        { (float)(margin * 2 + graphWidth), (float)(headerHeight + margin * 2 + graphHeight), (float)graphWidth, (float)graphHeight }
    };

    Rectangle statsPanel = {
        (float)(screenWidth - statsWidth - margin),
        (float)(headerHeight + margin),
        (float)statsWidth,
        (float)(screenHeight - headerHeight - 2 * margin)
    };

    // Convert populations to float once for plotting
    std::vector<float> populationsFloat;
    populationsFloat.reserve(data.populations.size());
    for (int p : data.populations) populationsFloat.push_back((float)p);

    // Precompute header strings
    const char* headerTitle = "Grub Simulation - Results Analysis";
    std::string summaryLine;
    if (!data.times.empty()) summaryLine = TextFormat("Total Samples: %d | Duration: %.1fs", (int)data.times.size(), data.times.back());

    int hoveredIndex = -1;
    Vector2 mousePos = {0,0};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{15, 15, 25, 255});

        // Header
        DrawRectangle(0, 0, screenWidth, headerHeight, Color{20, 20, 35, 255});
        DrawLine(0, headerHeight, screenWidth, headerHeight, Color{60, 60, 70, 255});
        DrawText(headerTitle, screenWidth/2 - MeasureText(headerTitle, 32)/2, 20, 32, LIGHTGRAY);
        if (!summaryLine.empty()) DrawText(summaryLine.c_str(), screenWidth/2 - MeasureText(summaryLine.c_str(), 16)/2, 65, 16, GRAY);

        if (!data.times.empty()) {
            hoveredIndex = -1;

            // Draw 4 graphs
            DrawGraph(graphAreas[0], "Population Over Time", data.times, populationsFloat, RED, "Time (seconds)", "Population");
            DrawGraph(graphAreas[1], "Average Speed Evolution", data.times, data.avgSpeeds, Color{0,200,0,255}, "Time (seconds)", "Speed");
            DrawGraph(graphAreas[2], "Average Sight Evolution", data.times, data.avgSights, SKYBLUE, "Time (seconds)", "Sight");
            DrawGraph(graphAreas[3], "Average Mutation Rate", data.times, data.avgMutations, Color{200,100,255,255}, "Time (seconds)", "Mutation %");

            // Mouse hover handling (binary search nearest time)
            mousePos = GetMousePosition();
            int hoveredGraph = -1;
            for (int i = 0; i < 4; ++i) if (CheckCollisionPointRec(mousePos, graphAreas[i])) { hoveredGraph = i; break; }

            if (hoveredGraph >= 0) {
                // Map mouse x to time fraction within inner graph area
                Rectangle inner = {
                    graphAreas[hoveredGraph].x + 60.0f,
                    graphAreas[hoveredGraph].y + 35.0f,
                    graphAreas[hoveredGraph].width - 60.0f - 20.0f,
                    graphAreas[hoveredGraph].height - 35.0f - 40.0f
                };
                float mouseRel = clampVal((mousePos.x - inner.x) / inner.width, 0.0f, 1.0f);
                float targetTime = data.times.front() + mouseRel * (data.times.back() - data.times.front());
                auto it = std::lower_bound(data.times.begin(), data.times.end(), targetTime);
                size_t idx = (it == data.times.end()) ? data.times.size() - 1 : std::distance(data.times.begin(), it);
                hoveredIndex = (int)idx;
            }

            // Statistics panel rendering
            DrawRectangleRec(statsPanel, Color{0,0,0,180});
            DrawRectangleLinesEx(statsPanel, 2, Color{60,60,70,255});
            DrawText("Statistics", (int)statsPanel.x + 15, (int)statsPanel.y + 15, 22, WHITE);

            int yPos = (int)statsPanel.y + 50;
            DrawText("Final Values:", (int)statsPanel.x + 15, yPos, 18, YELLOW); yPos += 30;

            // Final values
            DrawText(TextFormat("Population: %d", data.populations.back()), (int)statsPanel.x + 15, yPos, 16, LIGHTGRAY); yPos += 25;
            DrawText(TextFormat("Speed: %.1f", data.avgSpeeds.back()), (int)statsPanel.x + 15, yPos, 16, Color{0,200,0,255}); yPos += 25;
            DrawText(TextFormat("Sight: %.1f", data.avgSights.back()), (int)statsPanel.x + 15, yPos, 16, SKYBLUE); yPos += 25;
            DrawText(TextFormat("Mutation: %.1f%%", data.avgMutations.back()), (int)statsPanel.x + 15, yPos, 16, Color{200,100,255,255}); yPos += 40;

            // Range statistics (min/max pop)
            int maxPop = 0;
            int minPop = std::numeric_limits<int>::max();
            for (int p : data.populations) {
                maxPop = std::max(maxPop, p);
                minPop = std::min(minPop, p);
            }
            DrawText("Range Statistics:", (int)statsPanel.x + 15, yPos, 18, YELLOW); yPos += 30;
            DrawText(TextFormat("Max Population: %d", maxPop), (int)statsPanel.x + 15, yPos, 16, LIGHTGRAY); yPos += 25;
            DrawText(TextFormat("Min Population: %d", minPop), (int)statsPanel.x + 15, yPos, 16, LIGHTGRAY); yPos += 25;

            // Average values across the sampled points
            float avgSpeed = 0.0f, avgSight = 0.0f;
            size_t avgCount = data.avgSpeeds.size();
            for (size_t i = 0; i < avgCount; ++i) {
                avgSpeed += data.avgSpeeds[i];
                avgSight += data.avgSights[i];
            }
            if (avgCount > 0) {
                avgSpeed /= (float)avgCount;
                avgSight /= (float)avgCount;
                DrawText(TextFormat("Avg Speed: %.1f", avgSpeed), (int)statsPanel.x + 15, yPos, 16, LIGHTGRAY); yPos += 25;
                DrawText(TextFormat("Avg Sight: %.1f", avgSight), (int)statsPanel.x + 15, yPos, 16, LIGHTGRAY); yPos += 25;
            }

            // Legend
            yPos = (int)statsPanel.y + (int)statsPanel.height - 140;
            DrawText("Legend:", (int)statsPanel.x + 15, yPos, 18, WHITE); yPos += 25;
            DrawRectangle((int)statsPanel.x + 15, yPos, 12, 12, RED);
            DrawText("Population", (int)statsPanel.x + 35, yPos - 2, 16, RED); yPos += 22;
            DrawRectangle((int)statsPanel.x + 15, yPos, 12, 12, Color{0,200,0,255});
            DrawText("Speed", (int)statsPanel.x + 35, yPos - 2, 16, Color{0,200,0,255}); yPos += 22;
            DrawRectangle((int)statsPanel.x + 15, yPos, 12, 12, SKYBLUE);
            DrawText("Sight", (int)statsPanel.x + 35, yPos - 2, 16, SKYBLUE); yPos += 22;
            DrawRectangle((int)statsPanel.x + 15, yPos, 12, 12, Color{200,100,255,255});
            DrawText("Mutation", (int)statsPanel.x + 35, yPos - 2, 16, Color{200,100,255,255});

            // Hover tooltip with details for the nearest sample point
            if (hoveredIndex >= 0 && hoveredIndex < (int)data.times.size()) {
                float tooltipX = mousePos.x + 20;
                float tooltipY = mousePos.y + 20;
                const float tooltipWidth = 250;
                const float tooltipHeight = 120;
                if (tooltipX + tooltipWidth > screenWidth) tooltipX = mousePos.x - tooltipWidth - 10;
                if (tooltipY + tooltipHeight > screenHeight) tooltipY = mousePos.y - tooltipHeight - 10;

                DrawRectangle(tooltipX, tooltipY, tooltipWidth, tooltipHeight, Color{0,0,0,230});
                DrawRectangleLinesEx({tooltipX, tooltipY, tooltipWidth, tooltipHeight}, 2, WHITE);

                DrawText("Data Point Details:", (int)tooltipX + 10, (int)tooltipY + 10, 16, YELLOW);
                DrawText(TextFormat("Time: %.1fs", data.times[hoveredIndex]), (int)tooltipX + 10, (int)tooltipY + 35, 14, WHITE);
                DrawText(TextFormat("Population: %d", data.populations[hoveredIndex]), (int)tooltipX + 10, (int)tooltipY + 55, 14, RED);
                DrawText(TextFormat("Speed: %.1f", data.avgSpeeds[hoveredIndex]), (int)tooltipX + 10, (int)tooltipY + 75, 14, Color{0,200,0,255});
                DrawText(TextFormat("Sight: %.1f", data.avgSights[hoveredIndex]), (int)tooltipX + 10, (int)tooltipY + 95, 14, SKYBLUE);
            }
        } else {
            DrawText("No simulation data available to display.", screenWidth/2 - MeasureText("No simulation data available to display.", 24)/2, screenHeight/2, 24, RED);
            DrawText("Run a simulation first and save data to CSV.", screenWidth/2 - MeasureText("Run a simulation first and save data to CSV.", 18)/2, screenHeight/2 + 40, 18, GRAY);
        }

        DrawText("Move mouse over graphs to see data points | Press ESC to exit", screenWidth/2 - MeasureText("Move mouse over graphs to see data points | Press ESC to exit", 16)/2, screenHeight - 30, 16, GRAY);

        EndDrawing();
    }

    CloseWindow();
}


int main() {
    // RNG initialization
    initRNG();

    // Gather configuration from user via dialog
    int initialGrubs = 0, initialFood = 0, initialWater = 0, envWidth = 0, envHeight = 0;
    float mutationChance = 0.0f;
    if (!ShowConfigDialog(initialGrubs, initialFood, initialWater, mutationChance, envWidth, envHeight)) {
        return 0;
    }

    // Main simulation window
    InitWindow(envWidth, envHeight, "Grub Simulation - Press ESC to stop and view graphs");
    SetTargetFPS(60);

    Simulation sim;
    sim.initialGrubs = initialGrubs;
    sim.initialFood  = initialFood;
    sim.initialWater = initialWater;
    sim.baseMutationChance = mutationChance;
    sim.Init(envWidth, envHeight);

    float accumulator = 0.0f;
    int currentMaxGrubs = static_cast<int>((float(envWidth) * envHeight) / Config::AREA_PER_GRUB);

    // Main loop: input -> fixed-step updates -> rendering
    while (!WindowShouldClose() && !sim.simulationEnded) {
        // Input (controls)
        if (IsKeyPressed(KEY_SPACE)) sim.paused = !sim.paused;
        if (IsKeyPressed(KEY_R)) { sim.Init(envWidth, envHeight); accumulator = 0.0f; }
        if (IsKeyPressed(KEY_D)) sim.debugDraw = !sim.debugDraw;
        if (IsKeyPressed(KEY_EQUAL)) sim.timeScale = clampVal(sim.timeScale + 0.25f, 0.1f, 8.0f);
        if (IsKeyPressed(KEY_MINUS)) sim.timeScale = clampVal(sim.timeScale - 0.25f, 0.1f, 8.0f);
        if (IsKeyPressed(KEY_S) && sim.paused) sim.UpdateFixedStep();
        if (IsKeyPressed(KEY_ESCAPE)) sim.StopSimulation();

        // Fixed timestep accumulation pattern
        float realDt = GetFrameTime();
        if (!sim.paused) {
            accumulator += realDt * sim.timeScale;
            accumulator = clampVal(accumulator, 0.0f, 0.5f); // cap to avoid spiral of death
            while (accumulator >= Config::FIXED_STEP) {
                sim.UpdateFixedStep();
                accumulator -= Config::FIXED_STEP;
            }
        }
        if (sim.paused) {
        DrawText("Hover over grubs to see details", 16, 166, 14, LIME);
        }   
        // Rendering
        BeginDrawing();
        ClearBackground(Color{10, 10, 30, 255});
        sim.Draw();

        // UI overlay
        DrawRectangle(8, 8, 380, 160, Color{0,0,0,180});
        DrawText(TextFormat("Grubs: %d/%d", sim.GetGrubCount(), currentMaxGrubs), 16, 12, 20, LIME);
        DrawText(TextFormat("Food: %d  Water: %d", sim.GetFoodCount(), sim.GetWaterCount()), 16, 36, 18, YELLOW);
        DrawText(TextFormat("TimeScale: %.2fx (+/-)", sim.timeScale), 16, 58, 18, LIGHTGRAY);
        DrawText(TextFormat("Paused (SPACE): %s", sim.IsPaused() ? "YES" : "NO"), 16, 80, 18, LIGHTGRAY);
        DrawText(TextFormat("Time: %.1fs", sim.GetTotalTime()), 16, 102, 18, LIGHTGRAY);
        DrawText(TextFormat("Crowding Factor: %d", currentMaxGrubs), 16, 124, 16, ORANGE);
        DrawText("ESC=Stop&Save | Space=Pause | R=Restart | +/-=Speed", 16, 144, 12, GRAY);

        EndDrawing();
    }

    // Save and show graphs
    sim.SaveDataToFile("simulation_data.csv");
    CloseWindow();

    GraphData graphData = LoadGraphDataFromCSV("simulation_data.csv");
    if (!graphData.times.empty()) {
        ShowGraphs(graphData);
    } else {
        std::cout << "No simulation data to display." << std::endl;
    }

    return 0;
}
