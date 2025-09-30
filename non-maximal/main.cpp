#include <math.h>
#include <shmem.h>
extern "C" {
#include <spmat.h>
}
#include <std_options.h>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <fstream>
#include <sys/stat.h>
#include "selector.h"
#include <endian.h>
#include <ctime> 
#include <cstdlib> 
#include <random>
#include <cmath>
#include <cstddef>
#include <limits>
#include <chrono>
#include <utility>
#include <memory>
#include <sstream>
#include <stdint.h>

#include "graph.h"

int main(int argc, char *argv[]) {
    static long lock = 0;
    const char *deps[] = { "system", "bale_actor" };
    hclib::launch(deps, 2, [=] {
        T0_fprintf(stderr, "Non-maximal count of edges in MEL\n");
        std::string filename, matching_filename;
        int opt;
        while ((opt = getopt(argc, argv, "f:o:")) != -1) {
            switch (opt) {
                case 'f':
                    filename = optarg;
                    break;
                case 'o':
                    matching_filename = optarg;
                    break;
                default:
                    T0_fprintf(stderr, "Usage: -f <filename>\n");
                    exit(-1);
            }
        }
        /* Load the matching file in the set*/
        std::vector<VERTEX> output;
        std::ifstream file(matching_filename);
        std::string line;
        while (getline(file, line)) {
            VERTEX u;
            std::stringstream ss(line);
            ss >> u;
            output.push_back(u);
        }
        std::sort(output.begin(), output.end());
        output.erase(std::unique(output.begin(), output.end()), output.end());
        file.close();

        /* Read the file in distributed fashion and increment wherever the edge with both endpoints notin output is found*/
        graph g;
        uint64_t inc = g.init(filename, output);
        uint64_t glb_inc = lgp_reduce_add_l(inc);
        T0_fprintf(stderr, "Number of edges violated: %ld\n", glb_inc);
    });
    lgp_finalize();
    return EXIT_SUCCESS;
}