#define VERTEX uint64_t
#define WEIGHT double
#define EDGE std::pair<VERTEX, WEIGHT>

/* 
    Graph is assumed to contain both (u,v,w) and (v,u,w) in the file. 
    It assumes undirected, weighted graphs only 
*/

class graph {
    public:
        int init(std::string filename, std::vector<VERTEX> &output);
};

void stats(std::string filename) {
    std::ifstream file(filename);
    std::string line;
    getline(file, line);
    assert(line[0] == '%');
    getline(file, line); 
    std::stringstream ss(line);
    uint64_t a,b,c;
    ss >> a >> b >> c;
    assert(a == b);
    T0_fprintf(stderr, "Total Edges: %ld\n", c);
}

class graphPkt {
    public:
        VERTEX u;
        VERTEX v;
        WEIGHT w;
};

int graph::init(std::string filename, std::vector<VERTEX> &output) {
    stats(filename);
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

    uint64_t inc = 0;
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
        bool u_in = std::binary_search(output.begin(), output.end(), pkt.u);
        bool v_in = std::binary_search(output.begin(), output.end(), pkt.v);
        if (!u_in && !v_in) inc += 1;
    }
    file.close();
    return inc;
}