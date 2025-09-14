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
#include <cstdio>

#define VERTEX uint64_t
#define WEIGHT double
#define EDGE std::pair<VERTEX, WEIGHT>

/* 
    Graph is assumed to contain both (u,v,w) and (v,u,w) in the file. 
    It assumes undirected, weighted graphs only 
*/

class graph {
    public:
        uint64_t numberOfVertices = 0;
        uint64_t numberOfEdges = 0;
        std::unordered_map<VERTEX, std::unordered_map<VERTEX, WEIGHT>> adj;

        void init(std::string filename, std::string outname);
        void readGraph(std::string filename);
        void statsOfGraph(std::string filename, std::string outname);
        void write_mtx(uint64_t nverts, uint64_t nedges_twice, std::string outname);
        VERTEX _to_local(VERTEX u) {
            return u/THREADS;
        }
        VERTEX _to_global(VERTEX ll_u) {
            return MYTHREAD + ll_u*THREADS;
        }
        uint64_t _owner(VERTEX u) {
            return u%THREADS;
        }
};

void graph::init(std::string filename, std::string outname) {
    std::ifstream file(filename);
    std::string line;
    getline(file, line);
    assert(line[0] == '%');
    getline(file, line); 
    std::stringstream ss(line);
    uint64_t a,b,c;
    ss >> a >> b >> c;
    assert(a == b);
    numberOfVertices = a;
    numberOfEdges = c;
    readGraph(filename);
    statsOfGraph(filename, outname);
}

void graph::write_mtx(uint64_t nverts, uint64_t nedges_twice, std::string outname) {
    if (MYTHREAD == 0) {
        std::ofstream hout(outname, std::ios::out | std::ios::trunc);
        if (!hout) {
            T0_fprintf(stderr, "ERROR: cannot open %s for writing\n", outname.c_str());
            abort();
        }
        hout << "% MatrixMarket matrix coordinate real general\n";
        hout << nverts << " " << nverts << " " << nedges_twice << "\n";
        hout.close();
    }

    lgp_barrier(); 
    for (uint64_t t = 0; t < THREADS; ++t) {
        if (MYTHREAD == t) {
            FILE *fp = std::fopen(outname.c_str(), "ab");
            if (!fp) {
                T0_fprintf(stderr, "ERROR: cannot open %s for appending\n", outname.c_str());
                abort();
            }
            static const size_t BUF_SZ = size_t(64) << 20;
            std::unique_ptr<char[]> ubuf(new char[BUF_SZ]);
            std::setvbuf(fp, ubuf.get(), _IOFBF, BUF_SZ);
            for (const auto &kv_u : adj) {
                const VERTEX u = kv_u.first;
                const auto &nbrs = kv_u.second;

                for (const auto &kv_vw : nbrs) {
                    const VERTEX v = kv_vw.first;
                    const WEIGHT w = kv_vw.second;
                    std::fprintf(fp, "%llu %llu %.12f\n",
                                 (unsigned long long)(u + 1),
                                 (unsigned long long)(v + 1),
                                 (double)w);
                    std::fprintf(fp, "%llu %llu %.12f\n",
                                 (unsigned long long)(v + 1),
                                 (unsigned long long)(u + 1),
                                 (double)w);
                }
            }
            std::fflush(fp);
            std::fclose(fp);
        }
        lgp_barrier();
    }
}


inline void graph::statsOfGraph(std::string filename,  std::string outname) {
    T0_fprintf(stderr, "File %s\n", filename.c_str());
    uint64_t totallocEdges = 0;
    for(auto x: adj) {
        totallocEdges += x.second.size();
    }
    uint64_t totalEdges = lgp_reduce_add_l(totallocEdges);
    T0_fprintf(stderr, "Writing %s\n", outname.c_str());
    write_mtx(numberOfVertices, 2*totalEdges, outname);
}

class graphPkt {
    public:
        VERTEX u;
        VERTEX v;
        WEIGHT w;
};

class GraphSelector: public hclib::Selector<1, graphPkt> {
    graph *g;
    void process(graphPkt pkt, int sender_rank) {
        if(g->adj.find(pkt.u) == g->adj.end()) {
            std::unordered_map<VERTEX, WEIGHT> el;
            el.insert(std::make_pair(pkt.v, pkt.w));
            g->adj.insert(std::make_pair(pkt.u, el));
        }
        else {
            if(g->adj.find(pkt.u)->second.find(pkt.v) == g->adj.find(pkt.u)->second.end()) {
                g->adj.find(pkt.u)->second.insert(std::make_pair(pkt.v, pkt.w));
            }
            else {
                return;
            }
        }
    }

public:
    GraphSelector(graph *_g): hclib::Selector<1, graphPkt>(true), g(_g) {
        mb[0].process = [this](graphPkt pkt, int sender_rank) { this->process(pkt, sender_rank); };
    }
};

void graph::readGraph(std::string filename) {

    GraphSelector* readSelector = new GraphSelector(this);
    assert(numberOfVertices != 0 && numberOfEdges != 0);
    hclib::finish([=]() {
        struct stat stats;
        std::ifstream file(filename);
        std::string line;
        stat(filename.c_str(), &stats);

        uint64_t bytes = stats.st_size / THREADS;       
        uint64_t rem_bytes = stats.st_size % THREADS;
        uint64_t start, end;

        if(MYTHREAD < rem_bytes) {
            start = MYTHREAD*(bytes + 1);
            end = start + bytes + 1;
        }
        else {
            start = MYTHREAD*bytes + rem_bytes;
            end = start + bytes;
        }
        file.seekg(start);
        if (MYTHREAD != 0) {                                     
            file.seekg(start - 1);
            getline(file, line); 
            if (line[0] != '\n') start += line.size();         
        } 

        while (start < end && start < stats.st_size) {
            getline(file, line);
            start += line.size() + 1;
            if (line[0] == '#' || line[0] == '%') continue;
            graphPkt pkt;
            std::stringstream ss(line);
            ss >> pkt.u >> pkt.v >> pkt.w;
            if(pkt.u == pkt.v) continue; /*MTX first line automatically gets dropped!*/         
            pkt.u--;
            pkt.v--;
            assert(pkt.u < numberOfVertices && pkt.v < numberOfVertices);
            if(pkt.u > pkt.v) {
                std::swap(pkt.u, pkt.v);
            }
            readSelector->send(0, pkt, _owner(pkt.u));
        }
        file.close();
        readSelector->done(0);
    });
    delete readSelector;
}


int main(int argc, char *argv[]) {
    static long lock = 0;
    const char *deps[] = { "system", "bale_actor" };
    hclib::launch(deps, 2, [=] {
        std::string filename;
        std::string outname;
        int opt;
        while ((opt = getopt(argc, argv, "f:o:")) != -1) {
            switch (opt) {
                case 'f':
                    filename = optarg;
                    break;
                case 'o':
                    outname = optarg;
                    break;
                default:
                    T0_fprintf(stderr, "Usage: -f <filename>\n");
                    exit(-1);
            }
        }
        graph g;
        g.init(filename, outname);
        
    });
    lgp_finalize();
    return EXIT_SUCCESS;
}
