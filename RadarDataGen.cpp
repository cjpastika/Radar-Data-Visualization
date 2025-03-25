#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>

const int RADAR_RESOLUTION = 360;  // Degrees of sweep
const int RANGE_BINS = 100;         // Distance bins
const double MAX_RANGE = 10000.0;   // Maximum detection range in meters
const double NOISE_LEVEL = 0.1;     // Noise level for signal generation

struct RadarTarget {
    double distance;   // Distance from radar
    double angle;      // Angle of target
    double strength;   // Radar cross-section / signal strength
};

class RadarSignalGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> angle_dist{0.0, 360.0};
    std::uniform_real_distribution<> range_dist{0.0, MAX_RANGE};
    std::normal_distribution<> noise_dist{0.0, NOISE_LEVEL};

    std::vector<RadarTarget> generateTargets() {
        std::vector<RadarTarget> targets;
        
        // Increased target count
        int target_count = std::uniform_int_distribution<>(5, 10)(gen);
        
        for (int i = 0; i < target_count; ++i) {
            targets.push_back({
                range_dist(gen),           // Random distance
                angle_dist(gen),           // Random angle
                std::uniform_real_distribution<>(0.1, 0.7)(gen)  // Original strength range
            });
        }

        return targets;
    }

public:
    RadarSignalGenerator() : gen(rd()) {}

    void generateRadarData() {
        std::vector<std::vector<double>> radar_grid(RADAR_RESOLUTION, 
            std::vector<double>(RANGE_BINS, 0.0));
        
        auto targets = generateTargets();

        // More precise angular and range mapping
        for (int angle_bin = 0; angle_bin < RADAR_RESOLUTION; ++angle_bin) {
            double current_angle = angle_bin * (360.0 / RADAR_RESOLUTION);
            
            for (const auto& target : targets) {
                // More precise angle matching with some spread
                double angle_diff = std::abs(std::fmod(std::abs(target.angle - current_angle), 360.0));
                angle_diff = std::min(angle_diff, 360.0 - angle_diff);
                
                // Spread the signal across nearby bins
                if (angle_diff < 2.0) {
                    // Calculate range bin with some spread
                    int base_range_bin = static_cast<int>((target.distance / MAX_RANGE) * RANGE_BINS);
                    
                    // Spread signal across nearby range bins
                    for (int r_offset = -1; r_offset <= 1; ++r_offset) {
                        int range_bin = base_range_bin + r_offset;
                        if (range_bin >= 0 && range_bin < RANGE_BINS) {
                            // Spread signal strength based on proximity
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

        // Write to file
        std::ofstream file("radar_data.txt");
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
    }

    void continuousGeneration() {
        while (true) {
            generateRadarData();
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Update every 0.5 seconds
        }
    }
};

int main() {
    std::cout << "Starting radar data simulation...\n";
    RadarSignalGenerator generator;
    generator.generateRadarData();
    return 0;
}