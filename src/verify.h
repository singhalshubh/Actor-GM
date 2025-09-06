/*
    for any edge selected in matching u--v
        - delete adj[u] and adj[v]
    check for any w--u, (all remaining edges) whether 'u' or 'w' belongs to matching set?
*/

class verifyPkt {
    public:
    VERTEX u;
    VERTEX v;
    verifyPkt(VERTEX _u, VERTEX _v) {
        u = _u;
        v = _v;
    }
    verifyPkt() = default;
};

/*
    unique_u is ordered such that (a,b), a<b
*/

class VerifySelector: public hclib::Selector<1, verifyPkt> {
    graph &g;
    std::unordered_map<VERTEX, VERTEX> &unique_u;
    std::unordered_map<VERTEX, VERTEX> &unique_v;
    std::vector<VERTEX> &collection;

    void process(verifyPkt pkt, int sender_rank) {
        assert(pkt.u != pkt.v);
        if(pkt.u > pkt.v) {
            std::swap(pkt.u, pkt.v);
        }
        if(unique_u.find(pkt.u) == unique_u.end()) {
            unique_u.insert(std::make_pair(pkt.u, pkt.v));
            collection.push_back(pkt.u);
            collection.push_back(pkt.v);
        }
        else {
            assert(unique_u.find(pkt.u)->second == pkt.v);
        }
        if(unique_v.find(pkt.v) == unique_v.end()) {
            unique_v.insert(std::make_pair(pkt.v, pkt.u));
        }
        else {
            assert(unique_v.find(pkt.v)->second == pkt.u);
        }

    }

public:
    VerifySelector(graph &_g, std::unordered_map<VERTEX, VERTEX> &_unique_u, 
        std::unordered_map<VERTEX, VERTEX> &_unique_v, std::vector<VERTEX> &_collection): 
        hclib::Selector<1, verifyPkt>(true), g(_g), 
        unique_u(_unique_u), unique_v(_unique_v),
            collection(_collection) {
        mb[0].process = [this](verifyPkt pkt, int sender_rank) { this->process(pkt, sender_rank); };
    }
};

inline void _delete(VERTEX u, graph &g) {
    if(g._owner(u) == MYTHREAD) {
        g.adj[g._to_local(u)].clear();
    }
}

inline void _check(graph &g, std::vector<VERTEX> &collection) {
    for(const auto &el: g.adj) {
        for(const auto &edge: el) {
            assert(std::find(collection.begin(), collection.end(), edge.first) != collection.end());
        }
    }
}

bool _find_edge(VERTEX u, VERTEX v, graph &g) {
    if (g._owner(u) != MYTHREAD) return true;
        VERTEX ll_u = g._to_local(u);
        const auto &el = g.adj[ll_u];
        return std::find_if(el.begin(), el.end(),
                            [&](const auto &e){ return e.first == v; }) != el.end();
}

void verify(graph &g, std::vector<std::pair<VERTEX, VERTEX>> &final_set) {
    double t1 = wall_seconds();
    std::unordered_map<VERTEX, VERTEX> unique_u;
    std::unordered_map<VERTEX, VERTEX> unique_v;
    std::vector<VERTEX> collection;
    VerifySelector* verifySelector = new VerifySelector(g, unique_u, unique_v, collection);
    hclib::finish([&]() {
        for(auto edge: final_set) {
            for(uint64_t pe = 0; pe < THREADS; pe++) {
                verifyPkt pkt(edge.first, edge.second);
                verifySelector->send(0, pkt, pe);
            }
        }
        verifySelector->done(0);
    });
    delete verifySelector;
    assert(unique_u.size() == unique_v.size());
    assert(collection.size() == 2 * unique_u.size());
    T0_fprintf(stderr, "Matching Size: %ld\n", unique_u.size());
    T0_fprintf(stderr, "########## Verification ##########\n");
    T0_fprintf(stderr, "########## Step 1, unique matchings asserted ##########\n");
    for (const auto &edge : unique_u) {
        assert(_find_edge(edge.first, edge.second, g));
        assert(_find_edge(edge.second, edge.first, g));
    }
    T0_fprintf(stderr, "########## Step 2, all matched edges are in G(E) ##########\n");
    /*
        Note that map inherently checks for whether mate[u] = v || w is false. 
    */
    for(auto edge: unique_u) {    
        _delete(edge.first, g);
        _delete(edge.second, g);
    }
    _check(g, collection);
    T0_fprintf(stderr, "########## Step 3, graph after deletion has either endpoints in matching set ##########\n");
    T0_fprintf(stderr, "Time taken for verification: %.3f\n", wall_seconds() - t1);
}