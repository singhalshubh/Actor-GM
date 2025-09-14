// rgg_dump_mtx.cpp
// Write MTX ONLY, "as-is": no symmetry, no dedup, no weight checks.
// Builds the same RGG as your code (GenerateRGG), then dumps edges directly.
//
// mpicxx -O3 -std=c++17 rgg_dump_mtx.cpp -o rgg_dump_mtx

#include <mpi.h>
#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cassert>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <cstring>

#include "../mel-patched/graph.hpp"

static void write_mtx_as_is(const Graph* g, const std::string& mtxpath)
{
    MPI_Comm comm = g->get_comm();
    int rank; MPI_Comm_rank(comm, &rank);

    const GraphElem nv   = g->get_nv();
    const GraphElem lnv  = g->get_lnv();
    const GraphElem vbase= g->get_base(rank);

    // Count local edges exactly as stored (self-loops, duplicates, directions all preserved)
    GraphElem lnnz = 0;
    for (GraphElem i = 0; i < lnv; ++i) {
        GraphElem e0, e1; g->edge_range(i, e0, e1);
        lnnz += (e1 - e0);
    }

    // Global nnz
    GraphElem nnz = 0;
    MPI_Allreduce(&lnnz, &nnz, 1, MPI_GRAPH_TYPE, MPI_SUM, comm);

    // Build a single write buffer of my rows
    // Rough estimate: up to ~40–60 bytes per entry; reserve to avoid re-allocs.
    std::string mybuf;
    mybuf.reserve(static_cast<size_t>(std::max<GraphElem>(lnnz, 1)) * 56);

    for (GraphElem i = 0; i < lnv; ++i) {
        GraphElem e0, e1; g->edge_range(i, e0, e1);
        const long long u1 = static_cast<long long>(vbase + i) + 1; // 1-based for MTX
        for (GraphElem e = e0; e < e1; ++e) {
            const Edge& ed = g->get_edge(e);
            const long long v1 = static_cast<long long>(ed.tail_) + 1;
            const double w = static_cast<double>(ed.weight_);
            char line[128];
            // Format: "row col weight\n"
            int n = std::snprintf(line, sizeof(line), "%lld %lld %.12f\n", u1, v1, w);
            mybuf.append(line, static_cast<size_t>(n));
        }
    }

    // Header on rank 0
    std::ostringstream hdr;
    if (rank == 0) {
        hdr << "%%MatrixMarket matrix coordinate real general\n";
        hdr << nv << " " << nv << " " << nnz << "\n";
    }
    std::string header = hdr.str();
    MPI_Offset header_len = static_cast<MPI_Offset>(header.size());

    // Parallel write: header first, then ordered payload
    MPI_File fh;
    MPI_File_open(comm, mtxpath.c_str(),
                  MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    if (rank == 0 && header_len > 0) {
        MPI_File_write_at(fh, 0, (void*)header.data(),
                          static_cast<int>(header_len), MPI_BYTE, MPI_STATUS_IGNORE);
    }
    MPI_Barrier(comm);

    // Set view past the header; write in rank order (MPI_File_write_ordered)
    MPI_File_set_view(fh, header_len, MPI_BYTE, MPI_BYTE, (char*)"native", MPI_INFO_NULL);
    const int mysz = static_cast<int>(mybuf.size());
    if (mysz > 0) {
        MPI_File_write_ordered(fh, (void*)mybuf.data(), mysz, MPI_BYTE, MPI_STATUS_IGNORE);
    }
    MPI_File_close(&fh);
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // args: -n <nv> -o <out_prefix> [--lcg] [--random-pct P]
    GraphElem nv = 0;
    std::string out_prefix = "rgg";
    bool use_lcg = false;
    GraphWeight random_pct = 0.0;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-n" || a == "--nv") && i + 1 < argc) {
            nv = (GraphElem) std::atoll(argv[++i]);
        } else if ((a == "-o" || a == "--out") && i + 1 < argc) {
            out_prefix = argv[++i];
        } else if (a == "--lcg") {
            use_lcg = true;
        } else if (a == "--random-pct" && i + 1 < argc) {
            random_pct = (GraphWeight) std::atof(argv[++i]);
        }
    }

    if (nv <= 0) {
        if (rank == 0)
            std::fprintf(stderr,
                "Usage: %s -n <num_vertices> -o <out_prefix> [--lcg] [--random-pct P]\n",
                argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Generate the same RGG, with Euclidean weights (unitEdgeWeight=false)
    GenerateRGG gen(nv, MPI_COMM_WORLD);
    Graph* g = gen.generate(/*isLCG=*/use_lcg, /*unitEdgeWeight=*/false,
                            /*randomEdgePercent=*/random_pct);

    const std::string mtx = out_prefix + ".mtx";
    write_mtx_as_is(g, mtx);

    if (rank == 0) {
        std::cout << "Wrote MTX (as-is): " << mtx << std::endl;
    }

    delete g;
    MPI_Finalize();
    return 0;
}
