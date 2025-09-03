#include <math.h>
#include <shmem.h>
extern "C" {
#include <spmat.h>
}
#include <std_options.h>
#include <string>
#include <vector>
#include <queue>
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
#include "match.h"
#include "verify.h"

#define VERIFY 1

int main(int argc, char *argv[]) {
    static long lock = 0;
    const char *deps[] = { "system", "bale_actor" };
    hclib::launch(deps, 2, [=] {
        T0_fprintf(stderr, "Graph Matching 1/2-Approx Actor Version\n");
        std::string filename;
        int opt;
        while ((opt = getopt(argc, argv, "f:")) != -1) {
            switch (opt) {
                case 'f':
                    filename = optarg;
                    break;
                default:
                    T0_fprintf(stderr, "Usage: -f <filename>\n");
                    exit(-1);
            }
        }
        graph g;
        g.init(filename);
        match m;
        m.init(g);
        m.matching(g);
        #if VERIFY
            graph g_verify;
            g_verify.init(filename);
            verify(g_verify, m.final_set);
        #endif
    });
    lgp_finalize();
    return EXIT_SUCCESS;
}