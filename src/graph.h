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
        std::vector<std::vector<EDGE>> adj;

        void init(std::string filename);
        void readGraph(std::string filename);
        void statsOfGraph(std::string filename);
        void sortAdj();
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

void graph::init(std::string filename) {
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
    uint64_t ssize = numberOfVertices/THREADS + 1;
    adj.resize(ssize);
    double start = wall_seconds();
    readGraph(filename);
    T0_fprintf(stderr, "Reading file takes %.3lf seconds\n", wall_seconds() - start);
    statsOfGraph(filename);
    start = wall_seconds();
    sortAdj();
    T0_fprintf(stderr, "Sorting el for all G(V) takes %.3lf seconds\n", wall_seconds() - start);
}

inline void graph::statsOfGraph(std::string filename) {
    T0_fprintf(stderr, "File %s\n", filename.c_str());
    uint64_t totallocEdges = 0;
    for(auto x: adj) {
        totallocEdges += x.size();
    }
    uint64_t totalEdges = lgp_reduce_add_l(totallocEdges);
    T0_fprintf(stderr, "Number of Vertices: %ld\n", numberOfVertices);
    T0_fprintf(stderr, "Number of Edges: %ld\n", totalEdges);
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
        g->adj[g->_to_local(pkt.u)].push_back(std::make_pair(pkt.v, pkt.w));
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
            readSelector->send(0, pkt, _owner(pkt.u));
        }
        file.close();
        readSelector->done(0);
    });
    delete readSelector;
}

void graph::sortAdj() {
    auto cmp = [](const EDGE& a, const EDGE& b) {
        return (a.second < b.second) || (a.second == b.second && a.first > b.first);
    };
    for (auto& edges : adj) std::sort(edges.begin(), edges.end(), cmp);
}