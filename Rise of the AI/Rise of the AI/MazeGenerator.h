
#pragma once
#include <vector>

class MazeGenerator {
public:
    // Constructor, Initialize with maze dimensions
    MazeGenerator(int width, int height);

    // Generates the maze
    void generate_maze();

    // Convert generated maze to 1D array for LEVELA_DATA
    unsigned int* convert_to_level_data();

private:
    int width;
    int height;

    // 2D vector to hold the maze grid
    std::vector<std::vector<int>> maze;

    // Struct for holding cell positions
    struct Cell {
        int x, y;
    };
};
