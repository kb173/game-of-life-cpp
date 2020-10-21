#include <iostream>
#include <fstream>
#include <climits>
#include <vector>
#include <map>
#include <algorithm>

#define LIVE_CELL 'x'
#define DEAD_CELL '.'

struct World {
    World(int size_x, int size_y) : size_x(size_x), size_y(size_y) {
        data = new char*[size_y];

        for (int y = 0; y < size_y; y++) {
            data[y] = new char[size_x];
        }
    }

    ~World() {
        for (int y = 0; y < size_y; y++) {
            delete data[y];
        }
        
        delete data;
    }

    char **data;
    
    char get_value(int x, int y) {
        if (x < 0) x += size_x;
        if (y < 0) y += size_y;
        if (x >= size_x) x -= size_x;
        if (y >= size_y) y -= size_y;
        return data[y][x];
    }

    void set_alive(int x, int y) {
        data[y][x] = LIVE_CELL;
    }

    void set_dead(int x, int y) {
        data[y][x] = DEAD_CELL;
    }

    void set(int x, int y, char val) {
        data[y][x] = val;
    }

    int size_x;
    int size_y;
};

void generation(World &world) {
    // Set neighbor counts
    int **neighbor_counts = new int*[world.size_y];
    for (int y = 0; y < world.size_y; y++) {
        neighbor_counts[y] = new int[world.size_x];
    }

    for (int y = 0; y < world.size_y; y++) {
        for (int x = 0; x < world.size_x; x++) {
            // Get number of living neighbors
            int neighbors = 0;

            for (int local_y = -1; local_y < 2; local_y++) {
                for (int local_x = -1; local_x < 2; local_x++) {
                    // Ignore self
                    if (local_x == 0 && local_y == 0) continue;

                    if (world.get_value(x + local_x, y + local_y) == LIVE_CELL) {
                        neighbors++;
                    }
                }
            }

            neighbor_counts[y][x] = neighbors;
        }
    }

    // Update cells accordingly
    for (int y = 0; y < world.size_y; y++) {
        for (int x = 0; x < world.size_x; x++) {
            char this_cell = world.get_value(x, y);
            int neighbors = neighbor_counts[y][x];

            if (this_cell == DEAD_CELL) {
                if (neighbors == 3) {
                    // Any dead cell with exactly three living neighbors becomes a live cell.
                    world.set_alive(x, y);
                }
            } else {
                if (neighbors < 2 || neighbors > 3) {
                    // Any live cell with two or three living neighbors lives.
                    // Any live cell with fewer than two living neighbors dies.
                    // Any live cell with more than three living neighbors dies.
                    world.set_dead(x, y);
                }
            }
        }
    }
}

int main() {
    // Read in the start state
    std::ifstream world_file;
    world_file.open("random250_in.gol");

    // Get x and y size
    std::string x_str, y_str;
    getline(world_file, x_str, ',');
    getline(world_file, y_str);

    int size_x = std::stoi(x_str);
    int size_y = std::stoi(y_str);

    World world(size_x, size_y);

    // Set the data
    for (int y = 0; y < size_y; y++) {
        std::string line;
        getline(world_file, line);

        for (int x = 0; x < size_x; x++) {
            world.set(x, y, line[x]);
        }
    }
    
    world_file.close();

    // Do some generations
    for (int i = 0; i < 250; i++) {
        generation(world);
    }

    // Write the result
    std::ofstream result_file;
    result_file.open("random250_out.gol");

    result_file << size_x << "," << size_y << '\n';
    
    for (int y = 0; y < size_y; y++) {
        std::string line;
        getline(world_file, line);

        for (int x = 0; x < size_x; x++) {
            line += world.get_value(x, y);
        }

        result_file << line << '\n';
    }

    result_file.close();

    return 0;
}
