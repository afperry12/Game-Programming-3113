
#include "MazeGenerator.h"
#include <stack>
#include <algorithm>
#include <cstdlib>
#include <ctime>

// Constructor to initialize maze dimensions, fill with walls
MazeGenerator::MazeGenerator(int width, int height) : width(width), height(height) {
    // Resize 2D vector to match width and height, fill w/ walls
    maze.resize(height, std::vector<int>(width, 1));
}

// Generate the maze using DFS algo
void MazeGenerator::generate_maze() {
    std::srand(std::time(0));

    // Use stack for cells of DFS
    std::stack<Cell> stack;

    // Start at (1, 1) (top left corner)
    Cell start = {1, 1};
    maze[start.y][start.x] = 0;
    stack.push(start);

    // Directions
    const int directions[4][2] = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}};

    // DFS loop
    while (!stack.empty()) {
        Cell current = stack.top();
        stack.pop();

        // Add randomness
        std::vector<int> dir_indices = {0, 1, 2, 3};
        std::random_shuffle(dir_indices.begin(), dir_indices.end());
        
        for (int i = 0; i < 4; i++) {
            int dir = dir_indices[i];
            int new_x = current.x + directions[dir][0];
            int new_y = current.y + directions[dir][1];

            // Is new position within bounds + a wall
            if (new_x > 0 && new_x < width - 1 && new_y > 0 && new_y < height - 1 && maze[new_y][new_x] == 1) {
                // Create path between current and new cell
                maze[new_y][new_x] = 0;
                maze[current.y + directions[dir][1] / 2][current.x + directions[dir][0] / 2] = 0;

                // Push new cell onto stack
                stack.push({new_x, new_y});
            }
        }
    }
}

// Convert maze to 1D array for use w/ LEVELA_DATA
unsigned int* MazeGenerator::convert_to_level_data() {
    unsigned int* level_data = new unsigned int[width * height];

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Convert maze values (1 for walls, 0 for empty)
            if (maze[y][x] == 0) {
                level_data[y * width + x] = 0;
            } else if (maze[y][x] == 1) {
                // Check the block above to determine if uncovered or covered
                if (y == 0 || maze[y - 1][x] == 0) {
                    // Uncovered block
                    level_data[y * width + x] = 1;
                } else {
                    // Covered block
                    level_data[y * width + x] = 2;
                }
            }
        }
    }

    return level_data;
}
