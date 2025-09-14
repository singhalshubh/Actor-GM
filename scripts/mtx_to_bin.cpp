#include <mpi.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

using GraphElem   = std::uint64_t;
using GraphWeight = double;

struct Edge { GraphElem tail_; GraphWeight weight_; };
static_assert(sizeof(Edge) == sizeof(GraphElem) + sizeof(GraphWeight), "Edge ABI mismatch");

static inline std::string upper(std::string s){
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}
static inline void ltrim(std::string &s){
    size_t i=0; while(i<s.size() && std::isspace((unsigned char)s[i])) ++i;
    if(i) s.erase(0,i);
}
static inline void rtrim(std::string &s){
    while(!s.empty() && (s.back()=='\r' || std::isspace((unsigned char)s.back()))) s.pop_back();
}
static inline bool is_blank(const std::string &s){
    for(unsigned char c: s) if(!std::isspace(c)) return false; return true;
}

// 1-D block helpers
static inline int owner_of(GraphElem u, GraphElem nv, int P){
    GraphElem base=nv/P, rem=nv%P, cut=(base+1)*rem;
    return (u<cut)? int(u/(base+1)) : int(rem + (u-cut)/base);
}
static inline GraphElem local_first(int r, GraphElem nv, int P){
    GraphElem base=nv/P, rem=nv%P;
    return (r<rem)? GraphElem(r)*(base+1) : GraphElem(rem)*(base+1) + GraphElem(r-rem)*base;
}
static inline GraphElem local_count(int r, GraphElem nv, int P){
    GraphElem base=nv/P, rem=nv%P;
    return (r<rem)? (base+1) : base;
}

static void read_mm_header_rank0(const std::string& fn,
                                 GraphElem& nv, GraphElem& nnz, MPI_Offset& data_off)
{
    std::ifstream ifs(fn, std::ios::in | std::ios::binary);
    if (!ifs) { std::cerr << "open failed: " << fn << "\n"; MPI_Abort(MPI_COMM_WORLD, 1); }

    std::string line;
    bool found_header = false;
    while (std::getline(ifs, line)) {
        std::string t = line;
        t.erase(t.begin(), std::find_if(t.begin(), t.end(),
                    [](unsigned char c){ return !std::isspace(c); }));
        if (t.empty()) continue;

        std::istringstream iss(t);
        std::string tok, last;
        while (iss >> tok) last = tok;
        if (last.empty()) continue;

        if (upper(last) == "GENERAL") {
            found_header = true;
            break;
        }
    }
    if (!found_header) {
        std::cerr << "Could not find MatrixMarket header with GENERAL type\n";
        MPI_Abort(MPI_COMM_WORLD, 2);
    }

    while (std::getline(ifs, line)) {
        line.erase(line.begin(), std::find_if(line.begin(), line.end(),
                    [](unsigned char c){ return !std::isspace(c); }));
        while (!line.empty() && (line.back() == '\r' || std::isspace((unsigned char)line.back())))
            line.pop_back();

        if (line.empty() || line[0] == '%') continue;

        unsigned long long ns=0, nt=0, ne=0;
        std::istringstream iss(line);
        if (!(iss >> ns >> nt >> ne)) {
            std::cerr << "bad size line: " << line << "\n";
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
        nv  = (GraphElem)ns;
        nnz = (GraphElem)ne;
        data_off = (MPI_Offset)ifs.tellg();
        return;
    }

    std::cerr << "no size line found\n";
    MPI_Abort(MPI_COMM_WORLD, 4);
}

static std::string read_chunk_lines(MPI_File fh, MPI_Offset fsize,
                                    MPI_Offset off, MPI_Offset len,
                                    MPI_Offset data_off) {
    if (off >= fsize || len <= 0) return {};
    const MPI_Offset end     = std::min(off + len, fsize);
    const bool is_first      = (off == data_off);
    const bool is_last       = (end == fsize);
    const MPI_Offset OVER    = 64 * 1024;

    MPI_Offset roff = is_first ? off : std::max(data_off, off - OVER);
    MPI_Offset rlen = end - roff;

    std::string buf; buf.resize((size_t)rlen);
    MPI_File_read_at_all(fh, roff, buf.data(), (int)rlen, MPI_CHAR, MPI_STATUS_IGNORE);

    size_t start = 0;
    if (!is_first) {
        size_t anchor = (size_t)(off - roff);
        size_t pos = (anchor == 0) ? 0 : anchor - 1;
        while (pos > 0 && buf[pos] != '\n') --pos;
        if (pos > 0 || (pos == 0 && buf[pos] == '\n')) start = pos + (buf[pos] == '\n');
    }
    size_t stop = buf.size();
    if (!is_last) {
        size_t pos = buf.size();
        while (pos > start && buf[pos - 1] != '\n') --pos;
        stop = pos;
    }
    if (stop <= start) return {};
    return buf.substr(start, stop - start);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank=0, P=1; MPI_Comm_rank(MPI_COMM_WORLD, &rank); MPI_Comm_size(MPI_COMM_WORLD, &P);

    if (argc < 3) {
        if (rank == 0) std::cerr << "Usage: " << argv[0] << " input_general.mtx output.bin\n";
        MPI_Finalize();
        return 1;
    }
    const std::string in_mtx  = argv[1];
    const std::string out_bin = argv[2];

    // ---- 1) Header ----
    GraphElem nv = 0, nnz_hdr = 0; MPI_Offset data_off = 0;
    if (rank == 0) read_mm_header_rank0(in_mtx, nv, nnz_hdr, data_off);
    MPI_Bcast(&nv, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&nnz_hdr, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data_off, 1, MPI_OFFSET, 0, MPI_COMM_WORLD);

    // ---- 2) Partition & read coordinates ----
    MPI_File fin;
    if (MPI_File_open(MPI_COMM_WORLD, in_mtx.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fin) != MPI_SUCCESS) {
        if (rank==0) std::cerr << "MPI_File_open failed for input\n";
        MPI_Abort(MPI_COMM_WORLD, 10);
    }
    MPI_Offset fsize=0; MPI_File_get_size(fin, &fsize);
    const MPI_Offset coord_bytes = fsize - data_off;
    const MPI_Offset base = coord_bytes / P;
    const MPI_Offset rem  = coord_bytes % P;
    const MPI_Offset my_bytes = base + ((rank < rem) ? 1 : 0);
    const MPI_Offset my_off   = data_off + base * rank + std::min<MPI_Offset>(rank, rem);

    std::string text = read_chunk_lines(fin, fsize, my_off, my_bytes, data_off);
    MPI_File_close(&fin);

    // ---- 3) Parse my chunk (robust to spaces/CRLF) ----
    auto ltrim = [](std::string &s){
        size_t i=0; while(i<s.size() && std::isspace((unsigned char)s[i])) ++i;
        if (i) s.erase(0,i);
    };
    auto rtrim = [](std::string &s){
        while(!s.empty() && (s.back()=='\r' || std::isspace((unsigned char) s.back()))) s.pop_back();
    };

    struct Tup { GraphElem u, v; GraphWeight w; };
    std::vector<Tup> parsed; parsed.reserve(text.size()/24);
    {
        std::istringstream iss(text); std::string line;
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            rtrim(line); ltrim(line);
            if (line.empty() || line[0] == '%') continue;
            unsigned long long u1=0, v1=0; double w=1.0;
            std::istringstream ls(line);
            if (!(ls >> u1 >> v1)) continue;
            if (!(ls >> w)) w = 1.0;
            if (u1 == 0 || v1 == 0) continue;            // MM is 1-based
            GraphElem u = (GraphElem)(u1 - 1);
            GraphElem v = (GraphElem)(v1 - 1);
            if (u >= nv || v >= nv) continue;
            parsed.push_back({u, v, (GraphWeight)w});
        }
    }

    // ---- 4) Redistribute by source owner (proper MPI datatype) ----
    struct Packed { GraphElem u, v; GraphWeight w; };
    std::vector<int> sendc(P,0);
    for (auto &t : parsed) sendc[ owner_of(t.u, nv, P) ]++;
    std::vector<int> sdisp(P,0);
    for (int i=1;i<P;++i) sdisp[i] = sdisp[i-1] + sendc[i-1];

    std::vector<Packed> sbuf( sdisp.back() + (P ? sendc.back() : 0) );
    std::vector<int> cur(P,0);
    for (auto &t : parsed) {
        int p = owner_of(t.u, nv, P);
        sbuf[ sdisp[p] + cur[p]++ ] = Packed{ t.u, t.v, t.w };
    }

    MPI_Datatype MPI_PACKED_EDGE;
    {
        Packed tmp; int bl[3] = {1,1,1}; MPI_Aint d[3], base;
        MPI_Get_address(&tmp,    &base);
        MPI_Get_address(&tmp.u,  &d[0]);
        MPI_Get_address(&tmp.v,  &d[1]);
        MPI_Get_address(&tmp.w,  &d[2]);
        d[0]-=base; d[1]-=base; d[2]-=base;
        MPI_Datatype ty[3] = { MPI_UNSIGNED_LONG_LONG, MPI_UNSIGNED_LONG_LONG, MPI_DOUBLE };
        MPI_Type_create_struct(3, bl, d, ty, &MPI_PACKED_EDGE);
        MPI_Type_commit(&MPI_PACKED_EDGE);
    }

    std::vector<int> recvc(P,0), rdisp(P,0);
    MPI_Alltoall(sendc.data(), 1, MPI_INT, recvc.data(), 1, MPI_INT, MPI_COMM_WORLD);
    for (int i=1;i<P;++i) rdisp[i] = rdisp[i-1] + recvc[i-1];
    std::vector<Packed> rbuf( rdisp.back() + (P ? recvc.back() : 0) );

    MPI_Alltoallv(sbuf.data(), sendc.data(), sdisp.data(), MPI_PACKED_EDGE,
                  rbuf.data(), recvc.data(), rdisp.data(), MPI_PACKED_EDGE,
                  MPI_COMM_WORLD);
    MPI_Type_free(&MPI_PACKED_EDGE);

    // ---- 5) Build local CSR (row order) ----
    GraphElem lo = local_first(rank, nv, P);
    GraphElem ln = local_count(rank, nv, P);

    std::vector<GraphElem> deg(ln, 0);
    for (auto &e : rbuf) if (e.u >= lo && e.u < lo + ln) deg[(size_t)(e.u - lo)]++;

    std::stable_sort(rbuf.begin(), rbuf.end(),
                     [](const Packed& a, const Packed& b){ return a.u < b.u; });

    std::vector<GraphElem> rowptr_local(ln + 1, 0);
    for (GraphElem i=0;i<ln;++i) rowptr_local[i+1] = rowptr_local[i] + deg[i];

    std::vector<Edge> edges_local(rbuf.size());
    {
        std::vector<GraphElem> cursor(ln, 0);
        for (auto &e : rbuf) {
            GraphElem u_loc = e.u - lo;
            GraphElem pos   = rowptr_local[(size_t)u_loc] + cursor[(size_t)u_loc]++;
            edges_local[(size_t)pos] = Edge{ e.v, e.w }; // 0-based dest, weight
        }
    }

    // ---- 6) Global counts & offsets ----
    GraphElem ne_local  = (GraphElem)edges_local.size();
    GraphElem ne_global = 0;
    MPI_Allreduce(&ne_local, &ne_global, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

    if (rank == 0 && nnz_hdr && nnz_hdr != ne_global) {
        std::cerr << "[warn] header nnz=" << nnz_hdr << " parsed=" << ne_global << "\n";
    }

    GraphElem edge_off_rank = 0;
    MPI_Exscan(&ne_local, &edge_off_rank, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
    if (rank == 0) edge_off_rank = 0;

    GraphElem rowptr_base = 0;
    GraphElem deg_sum_local = rowptr_local.back(); // == ne_local
    MPI_Exscan(&deg_sum_local, &rowptr_base, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
    if (rank == 0) rowptr_base = 0;

    // ---- 7) Write .bin (independent writes; fast close) ----
    const MPI_Offset HDR = sizeof(GraphElem)*2;
    const MPI_Offset RPT = (MPI_Offset)((nv + 1) * sizeof(GraphElem));

    MPI_File fout;
    MPI_Info info; MPI_Info_create(&info);
    MPI_Info_set(info, "romio_ds_write", "disable"); // optional: avoid data sieving
    MPI_File_open(MPI_COMM_WORLD, out_bin.c_str(),
                  MPI_MODE_CREATE | MPI_MODE_WRONLY, info, &fout);
    MPI_Info_free(&info);

    // header (rank 0)
    if (rank == 0) {
        MPI_File_write_at(fout, 0,               &nv,        1, MPI_UNSIGNED_LONG_LONG, MPI_STATUS_IGNORE);
        MPI_File_write_at(fout, sizeof(GraphElem), &ne_global, 1, MPI_UNSIGNED_LONG_LONG, MPI_STATUS_IGNORE);
    }

    // rowptr slice [lo .. lo+ln-1]
    if (ln > 0) {
        std::vector<GraphElem> slice(ln);
        for (GraphElem i=0;i<ln;++i) slice[(size_t)i] = rowptr_base + rowptr_local[(size_t)i];
        MPI_Offset off_row = HDR + (MPI_Offset)lo * sizeof(GraphElem);
        // write in one go (ln fits int in practice per-rank)
        MPI_File_write_at(fout, off_row,
                          slice.data(), (int)slice.size(),
                          MPI_UNSIGNED_LONG_LONG, MPI_STATUS_IGNORE);
    }
    // rowptr[nv] (only last rank)
    if (rank == P - 1) {
        GraphElem last = rowptr_base + rowptr_local.back(); // == ne_global
        MPI_Offset off_tail = HDR + (MPI_Offset)nv * sizeof(GraphElem);
        MPI_File_write_at(fout, off_tail, &last, 1,
                          MPI_UNSIGNED_LONG_LONG, MPI_STATUS_IGNORE);
    }

    // edges block
    if (!edges_local.empty()) {
        MPI_Offset off_edges = HDR + RPT + (MPI_Offset)edge_off_rank * sizeof(Edge);
        // if extremely large, you can chunk; otherwise single call is fine
        MPI_File_write_at(fout, off_edges,
                          edges_local.data(),
                          (int)(edges_local.size() * sizeof(Edge)),
                          MPI_BYTE, MPI_STATUS_IGNORE);
    }

    // optional: make the wait happen here (not at close/finalize)
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_File_close(&fout);
    MPI_Finalize();
    return 0;
}
