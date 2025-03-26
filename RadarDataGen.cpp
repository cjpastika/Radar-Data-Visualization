#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <algorithm>

const int RADAR_RESOLUTION = 360;
const int RANGE_BINS = 100;
const double MAX_RANGE = 10000.0;
const double NOISE_LEVEL = 0.2;

struct MovingTarget {
    double x;           // X coordinate
    double y;           // Y coordinate
    double velocity_x;  // X velocity
    double velocity_y;  // Y velocity
    double strength;    // Signal strength
};

class DynamicRadarSignalGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> angle_dist{0.0, 360.0};
    std::uniform_real_distribution<> range_dist{0.0, MAX_RANGE};
    std::normal_distribution<> noise_dist{0.0, NOISE_LEVEL};

    std::vector<MovingTarget> targets;

    void initializeTargets() {
        targets.clear();
        int target_count = std::uniform_int_distribution<>(8, 15)(gen);
        
        for (int i = 0; i < target_count; ++i) {
            targets.push_back(createEdgeTarget());
        }
    }

    MovingTarget createEdgeTarget() {
        // Randomly choose which edge to spawn at
        int edge = std::uniform_int_distribution<>(0, 3)(gen);
        
        double x, y, vx, vy;
        switch(edge) {
            case 0: // Left edge
                x = 0.0;
                y = range_dist(gen);
                vx = std::uniform_real_distribution<>(100, 500)(gen);
                vy = std::uniform_real_distribution<>(-200, 200)(gen);
                break;
            case 1: // Right edge
                x = MAX_RANGE;
                y = range_dist(gen);
                vx = -std::uniform_real_distribution<>(100, 500)(gen);
                vy = std::uniform_real_distribution<>(-200, 200)(gen);
                break;
            case 2: // Top edge
                x = range_dist(gen);
                y = 0.0;
                vx = std::uniform_real_distribution<>(-200, 200)(gen);
                vy = std::uniform_real_distribution<>(100, 500)(gen);
                break;
            case 3: // Bottom edge
                x = range_dist(gen);
                y = MAX_RANGE;
                vx = std::uniform_real_distribution<>(-200, 200)(gen);
                vy = -std::uniform_real_distribution<>(100, 500)(gen);
                break;
        }

        return {
            x,  // Initial X
            y,  // Initial Y
            vx, // X velocity 
            vy, // Y velocity
            std::uniform_real_distribution<>(0.3, 1.0)(gen) // Strength
        };
    }

    void updateTargets(double time_step) {
        // Use erase-remove idiom to delete targets that are out of bounds
        targets.erase(
            std::remove_if(targets.begin(), targets.end(), 
                [](const MovingTarget& target) {
                    // Return true if target is fully outside the graph
                    return target.x < 0 || target.x > MAX_RANGE || 
                           target.y < 0 || target.y > MAX_RANGE;
                }
            ), 
            targets.end()
        );

        // Update remaining targets' positions
        for (auto& target : targets) {
            // Update position with constant velocity
            target.x += target.velocity_x * time_step;
            target.y += target.velocity_y * time_step;
        }
    }

    std::vector<std::vector<double>> generateRadarGrid() {
        std::vector<std::vector<double>> radar_grid(RADAR_RESOLUTION, 
            std::vector<double>(RANGE_BINS, 0.0));
        
        for (int angle_bin = 0; angle_bin < RADAR_RESOLUTION; ++angle_bin) {
            double current_angle = angle_bin * (360.0 / RADAR_RESOLUTION);
            
            for (const auto& target : targets) {
                // Calculate target's polar coordinates
                double dx = target.x;
                double dy = target.y;
                double distance = std::sqrt(dx*dx + dy*dy);
                double angle = std::atan2(dy, dx) * 180.0 / M_PI;
                if (angle < 0) angle += 360.0;

                // More precise angle matching with some spread
                double angle_diff = std::abs(std::fmod(std::abs(angle - current_angle), 360.0));
                angle_diff = std::min(angle_diff, 360.0 - angle_diff);
                
                // Spread the signal across nearby bins
                if (angle_diff < 2.0) {
                    int base_range_bin = static_cast<int>((distance / MAX_RANGE) * RANGE_BINS);
                    
                    for (int r_offset = -1; r_offset <= 1; ++r_offset) {
                        int range_bin = base_range_bin + r_offset;
                        if (range_bin >= 0 && range_bin < RANGE_BINS) {
                            double spread_factor = 1.0 - std::abs(r_offset) * 0.3;
                            
                            radar_grid[angle_bin][range_bin] = std::max(
                                target.strength + noise_dist(gen), 
                                radar_grid[angle_bin][range_bin]
                            );
                        }
                    }
                }
            }
        }

        return radar_grid;
    }

public:
    DynamicRadarSignalGenerator() : gen(rd()) {
        // Ensure output directory exists
        std::filesystem::create_directory("radar_frames");
    }

    void generateRadarSequence(int num_frames = 300, double time_step = 0.05) {
        initializeTargets();

        for (int frame = 0; frame < num_frames; ++frame) {
            // Generate radar grid
            auto radar_grid = generateRadarGrid();

            // Corrected filename generation
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(4) << frame;
            std::string filename = "radar_frames/frame_" + ss.str() + ".txt";
            
            std::ofstream file(filename);
            if (!file) {
                std::cerr << "Error: Unable to open file for writing.\n";
                return;
            }

            // Write grid data to file
            for (const auto& angle_data : radar_grid) {
                for (const auto& value : angle_data) {
                    file << value << " ";
                }
                file << "\n";
            }
            file.close();

            // Update target positions
            updateTargets(time_step);

            // Add new targets at edges to replace removed ones
            if (targets.size() < 8) {
                int new_target_count = std::uniform_int_distribution<>(1, 5)(gen);
                for (int i = 0; i < new_target_count; ++i) {
                    targets.push_back(createEdgeTarget());
                }
            }
        }
    }
};

int main() {
    std::cout << "Starting dynamic radar data simulation...\n";
    DynamicRadarSignalGenerator generator;
    generator.generateRadarSequence();
    return 0;
}