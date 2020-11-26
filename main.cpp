#include <omp.h>
#include <fstream>
#include <iostream>

#include "Timing.h"

#define LIVE_CELL 1  // 'x' in the input data
#define DEAD_CELL 0  // '.' in the input data

enum Mode {
    SEQ,
    OMP,
    OCL
};

// Using this struct seems to be more performant than just passing
//  a bool** around functions. However, also adding the neighbor_count
//  made performance worse.
struct World {
    World(int size_x, int size_y) : size_x(size_x), size_y(size_y) {
        data = new bool*[size_y];

        for (int y = 0; y < size_y; y++) {
            data[y] = new bool[size_x];
        }
    }

    ~World() {
        for (int y = 0; y < size_y; y++) {
            delete data[y];
        }
        
        delete data;
    }

    bool **data;

    // All following functions are just convenience shorthands.
    // They are inlined so it doesn't make a difference in performance.
    
    inline bool get_value(int x, int y) {
        return data[y][x];
    }

    inline void set_alive(int x, int y) {
        data[y][x] = LIVE_CELL;
    }

    inline void set_dead(int x, int y) {
        data[y][x] = DEAD_CELL;
    }

    inline void set(int x, int y, bool val) {
        data[y][x] = val;
    }

    inline int get_num_neighbors(int left, int right, int up, int down, int x, int y) {
        return
            get_value(left, down) +
            get_value(x, down) +
            get_value(right, down) +
            get_value(left, y) +
            get_value(right, y) +
            get_value(left, up) +
            get_value(x, up) +
            get_value(right, up);
    }

    int size_x;
    int size_y;
};

void generation_omp(World &world, int *neighbor_counts) {
    // Shorthand to prevent always having to access via world
    int size_x = world.size_x;
    int size_y = world.size_y;

    // Set the neighbor count array according to the world.

    // We handle x == 0 and x == size_x - 1 separately in order to avoid all the constant if checks.
    int loop_x = size_x - 1;

    #pragma omp parallel for
    for (int y = 0; y < size_y; y++) {
        // Wrap y
        // This happens rarely enough that this if isn't a huge problem, and it would be tedious
        //  to handle both this and x manually.
        int up = y - 1;
        int down = y + 1;

        if (up < 0)
            up += size_y;
        else if (down >= size_y)
            down -= size_y;

        // Handle x == 0
        neighbor_counts[y * size_x + 0] = world.get_num_neighbors(loop_x, 1, up, down, 0, y);
        
        // Handle 'normal' x
        for (int x = 1; x < loop_x; x++) {
            neighbor_counts[y * size_x + x] = world.get_num_neighbors(x - 1, x + 1, up, down, x, y);
        }

        // Handle x == loop_x (== size_x - 1, we're just re-using the variable
        neighbor_counts[y * size_x + loop_x] = world.get_num_neighbors(loop_x - 1, 0, up, down, loop_x, y);
    }

    // Update cells accordingly
    #pragma omp parallel for
    for (int y = 0; y < world.size_y; y++) {
        for (int x = 0; x < world.size_x; x++) {
            char this_cell = world.get_value(x, y);
            int neighbors = neighbor_counts[y * size_x + x];

            world.data[y][x] = (neighbors == 3) + this_cell * (neighbors == 2);
        }
    }
}

void generation_seq(World &world, int *neighbor_counts) {
    // Shorthand to prevent always having to access via world
    int size_x = world.size_x;
    int size_y = world.size_y;

    // Set the neighbor count array according to the world.

    // We handle x == 0 and x == size_x - 1 separately in order to avoid all the constant if checks.
    int loop_x = size_x - 1;

    for (int y = 0; y < size_y; y++) {
        // Wrap y
        // This happens rarely enough that this if isn't a huge problem, and it would be tedious
        //  to handle both this and x manually.
        int up = y - 1;
        int down = y + 1;

        if (up < 0)
            up += size_y;
        else if (down >= size_y)
            down -= size_y;

        // Handle x == 0
        neighbor_counts[y * size_x + 0] = world.get_num_neighbors(loop_x, 1, up, down, 0, y);
        
        // Handle 'normal' x
        for (int x = 1; x < loop_x; x++) {
            neighbor_counts[y * size_x + x] = world.get_num_neighbors(x - 1, x + 1, up, down, x, y);
        }

        // Handle x == loop_x (== size_x - 1, we're just re-using the variable
        neighbor_counts[y * size_x + loop_x] = world.get_num_neighbors(loop_x - 1, 0, up, down, loop_x, y);
    }

    // Update cells accordingly
    for (int y = 0; y < world.size_y; y++) {
        for (int x = 0; x < world.size_x; x++) {
            char this_cell = world.get_value(x, y);
            int neighbors = neighbor_counts[y * size_x + x];

            world.data[y][x] = (neighbors == 3) + this_cell * (neighbors == 2);
        }
    }
}

void print_usage() {
    std::cerr << "Usage: gol --mode seq|omp|ocl [--threads number] [--device cpu|gpu] --load infile.gol --save outfile.gol --generations number [--measure]" << std::endl;
}

int main(int argc, char* argv[]) {
    Timing *timing = Timing::getInstance();

    // Setup.
    timing->startSetup();

    // Parse command line arguments
    std::string infile;
    std::string outfile;
    Mode mode = Mode::SEQ;
    bool use_gpu = false;
    int num_generations = 0;
    int num_threads = 1;
    bool measure = false;

    if (argc < 8) {
        print_usage();
        return 1;
    }

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--load") {
            if (i + 1 < argc) {
                infile = argv[i+1];
            } else {
                print_usage();
                return 1;
            }  
        } else if (std::string(argv[i]) == "--save") {
            if (i + 1 < argc) {
                outfile = argv[i+1];
            } else {
                print_usage();
                return 1;
            } 
        } else if (std::string(argv[i]) == "--mode") {
            if (i + 1 < argc) {
                if (std::string(argv[i+1]) == "seq") {
                    mode = Mode::SEQ;
                } else if (std::string(argv[i+1]) == "omp") {
                    mode = Mode::OMP;
                } else if (std::string(argv[i+1]) == "ocl") {
                    mode = Mode::OCL;
                } else {
                    print_usage();
                    return 1;
                }
            } else {
                print_usage();
                return 1;
            }
        } else if (std::string(argv[i]) == "--threads") {
            if (i + 1 < argc) {
                num_threads = std::stoi(argv[i+1]);
            } else {
                print_usage();
                return 1;
            }
        } else if (std::string(argv[i]) == "--device") {
            if (i + 1 < argc) {
                if (std::string(argv[i+1]) == "cpu") {
                    use_gpu = false;
                } else if (std::string(argv[i+1]) == "gpu") {
                    use_gpu = true;
                } else {
                    print_usage();
                    return 1;
                }
            } else {
                print_usage();
                return 1;
            }
        } else if (std::string(argv[i]) == "--generations") {
            if (i + 1 < argc) {
                num_generations = std::stoi(argv[i+1]);
            } else {
                print_usage();
                return 1;
            }  
        } else if (std::string(argv[i]) == "--measure") {
            measure = true;
        }
    }

    // TODO: Just for testing
    if (use_gpu) {
        std::cout << "Using GPU" << std::endl;
    } else {
        std::cout << "Using CPU" << std::endl;
    }

    if (mode == Mode::SEQ) {
        std::cout << "Running classic sequential version" << std::endl;
    } else if (mode == Mode::OMP) {
        std::cout << "Running OpenMP version with " << num_threads << " threads" << std::endl;
    } else if (mode == Mode::OCL) {
        std::cout << "Running OpenCL version" << std::endl;
    }

    // Read in the start state
    std::ifstream world_file;
    world_file.open(infile);

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
            // The chars '.' and 'x' are mapped to the booleans 0 and 1.
            // This speeds up the calculation of the neighbors -- no if-checks
            //  needed, just sum the values.
            world.set(x, y, 1 ? line[x] == 'x' : 0);
        }
    }
    
    world_file.close();

    // In this separate array, we keep track of how many live neighbors
    //  a certain cell has. This is because immediately updating based
    //  on the number of neighbors would mess with later calculations
    //  of adjacent cells.
    int *neighbor_counts = new int[world.size_y * world.size_x];

    timing->stopSetup();
    timing->startComputation();

    // Do some generations
    if (mode == Mode::SEQ) {
        for (int i = 0; i < num_generations; i++) {
            generation_seq(world, neighbor_counts);
        }
    } else if (mode == Mode::OMP) {
        for (int i = 0; i < num_generations; i++) {
            generation_omp(world, neighbor_counts);
        }
    }

    timing->stopComputation();
    timing->startFinalization();

    // Write the result
    std::ofstream result_file;
    result_file.open(outfile);

    result_file << size_x << "," << size_y << '\n';
    
    for (int y = 0; y < size_y; y++) {
        std::string line;
        getline(world_file, line);

        for (int x = 0; x < size_x; x++) {
            // Convert 1 and 0 to 'x' and '.' again
            line += world.get_value(x, y) ? 'x' : '.';
        }

        result_file << line << '\n';
    }

    result_file.close();
    delete neighbor_counts;

    timing->stopFinalization();

    if (measure) {
        std::cout << timing->getResults() << std::endl;
    }

    return 0;
}
