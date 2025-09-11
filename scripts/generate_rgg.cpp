#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <omp.h>

struct Args {
    uint64_t n = 0;
    double   d = 0.0;
    std::string out;
    uint64_t seed = 42;
};

static void die(const char* msg){ std::fprintf(stderr,"FATAL: %s\n",msg); std::exit(1); }

static Args parse(int argc, char** argv){
    Args a;
    for(int i=1;i<argc;i++){
        std::string s(argv[i]);
        auto need = [&](int i){ if(i+1>=argc) die("missing value"); return argv[i+1]; };
        if(s=="-n"||s=="--num-vertices") a.n = std::strtoull(need(i++),nullptr,10);
        else if(s=="-d"||s=="--density") a.d = std::atof(need(i++));
        else if(s=="-o"||s=="--output") a.out = need(i++);
        else if(s=="-s"||s=="--seed")   a.seed = std::strtoull(need(i++),nullptr,10);
        else if(s=="-h"||s=="--help"){
            std::puts("Usage: rgg_omp_mtx -n N -d D -o out.mtx [-s SEED]");
            std::exit(0);
        }
    }
    if(a.n<2 || !(a.d>0.0 && a.d<1.0) || a.out.empty())
        die("need -n >=2, -d in (0,1), -o PATH");
    return a;
}

int main(int argc, char** argv){
    Args args = parse(argc, argv);
    const double r  = std::sqrt(args.d/M_PI);
    const double r2 = r*r;

    // Generate positions (float to save RAM)
    std::vector<std::pair<float,float>> pos(args.n);
    {
        std::mt19937_64 gen(args.seed);
        std::uniform_real_distribution<double> U(0.0,1.0);
        for(uint64_t i=0;i<args.n;i++){
            pos[i].first  = static_cast<float>(U(gen));
            pos[i].second = static_cast<float>(U(gen));
        }
    }

    // Grid (cell size = r)
    const int nx = std::max(1, (int)std::ceil(1.0 / r));
    const int ny = nx;
    const double cell = 1.0 / nx;

    auto cell_id = [&](float x, float y)->int{
        int ix = std::min(nx-1, std::max(0, (int)std::floor(x / cell)));
        int iy = std::min(ny-1, std::max(0, (int)std::floor(y / cell)));
        return iy*nx + ix;
    };

    // Bin points into cells
    std::vector<std::vector<uint32_t>> cells(nx*ny);
    for(uint32_t i=0;i<pos.size();++i) cells[cell_id(pos[i].first,pos[i].second)].push_back(i);

    // Helper: iterate neighbor cells with lexicographic ordering to avoid double checks
    auto neighbor_cells = [&](int cx, int cy, std::vector<std::pair<int,int>>& out){
        out.clear();
        for(int dy=-1; dy<=1; ++dy){
            for(int dx=-1; dx<=1; ++dx){
                int nx_ = cx+dx, ny_ = cy+dy;
                if(nx_<0||nx_>=nx||ny_<0||ny_>=ny) continue;
                if(ny_ < cy) continue;
                if(ny_==cy && nx_ < cx) continue;
                out.emplace_back(nx_,ny_);
            }
        }
    };

    // Pass 1: count undirected edges (u<v)
    unsigned long long m = 0ULL;
    #pragma omp parallel
    {
        std::vector<std::pair<int,int>> neigh;
        #pragma omp for reduction(+:m) schedule(static)
        for(int cy=0; cy<ny; ++cy){
            for(int cx=0; cx<nx; ++cx){
                const auto& A = cells[cy*nx+cx];
                neighbor_cells(cx,cy,neigh);
                for(auto [nx0,ny0] : neigh){
                    const auto& B = cells[ny0*nx+nx0];
                    if (&A == &B){
                        for(size_t ia=0; ia<A.size(); ++ia){
                            const uint32_t i = A[ia];
                            const float xi = pos[i].first, yi = pos[i].second;
                            for(size_t jb=ia+1; jb<B.size(); ++jb){
                                const uint32_t j = B[jb];
                                const float dx = xi - pos[j].first;
                                const float dy = yi - pos[j].second;
                                if ((double)dx*dx + (double)dy*dy <= r2) m += 1ULL;
                            }
                        }
                    } else {
                        for(size_t ia=0; ia<A.size(); ++ia){
                            const uint32_t i = A[ia];
                            const float xi = pos[i].first, yi = pos[i].second;
                            for(size_t jb=0; jb<B.size(); ++jb){
                                const uint32_t j = B[jb];
                                const float dx = xi - pos[j].first;
                                const float dy = yi - pos[j].second;
                                if ((double)dx*dx + (double)dy*dy <= r2) m += 1ULL;
                            }
                        }
                    }
                }
            }
        }
    }
    const unsigned long long nnz = 2ULL * m;

    // Pass 2: write header + edges (both directions)
    std::FILE* f = std::fopen(args.out.c_str(),"w");
    if(!f) die("failed to open output");
    std::fprintf(f,"%%%%MatrixMarket matrix coordinate real general\n");
    std::fprintf(f,"%llu %llu %llu\n",
                 (unsigned long long)args.n,
                 (unsigned long long)args.n,
                 nnz);

    // Single-threaded write for simplicity (I/O bound anyway)
    std::vector<std::pair<int,int>> neigh;
    for(int cy=0; cy<ny; ++cy){
        for(int cx=0; cx<nx; ++cx){
            const auto& A = cells[cy*nx+cx];
            neighbor_cells(cx,cy,neigh);
            for(auto [nx0,ny0] : neigh){
                const auto& B = cells[ny0*nx+nx0];
                if (&A == &B){
                    for(size_t ia=0; ia<A.size(); ++ia){
                        const uint32_t i = A[ia];
                        const float xi = pos[i].first, yi = pos[i].second;
                        for(size_t jb=ia+1; jb<B.size(); ++jb){
                            const uint32_t j = B[jb];
                            const float dx = xi - pos[j].first;
                            const float dy = yi - pos[j].second;
                            const double d2 = (double)dx*dx + (double)dy*dy;
                            if (d2 <= r2){
                                const double w = std::sqrt(d2);
                                const uint64_t u = (uint64_t)i + 1, v = (uint64_t)j + 1;
                                std::fprintf(f, "%llu %llu %.9g\n%llu %llu %.9g\n",
                                             (unsigned long long)u, (unsigned long long)v, w,
                                             (unsigned long long)v, (unsigned long long)u, w);
                            }
                        }
                    }
                } else {
                    for(size_t ia=0; ia<A.size(); ++ia){
                        const uint32_t i = A[ia];
                        const float xi = pos[i].first, yi = pos[i].second;
                        for(size_t jb=0; jb<B.size(); ++jb){
                            const uint32_t j = B[jb];
                            const float dx = xi - pos[j].first;
                            const float dy = yi - pos[j].second;
                            const double d2 = (double)dx*dx + (double)dy*dy;
                            if (d2 <= r2){
                                const double w = std::sqrt(d2);
                                const uint64_t u = (uint64_t)std::min(i,j) + 1;
                                const uint64_t v = (uint64_t)std::max(i,j) + 1;
                                std::fprintf(f, "%llu %llu %.9g\n%llu %llu %.9g\n",
                                             (unsigned long long)u, (unsigned long long)v, w,
                                             (unsigned long long)v, (unsigned long long)u, w);
                            }
                        }
                    }
                }
            }
        }
    }
    std::fclose(f);
    return 0;
}
