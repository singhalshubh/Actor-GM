enum class state {
    init = 0,
    waiting = 1,
    done = 2
};

enum class comm {
    request = 0,
    accept = 1,
    reject = 2
};

class match {
    public:
        std::vector<state> status;
        std::vector<EDGE> storage;
        std::vector<std::pair<VERTEX, VERTEX>> final_set;
        void init(graph &g);
        void matching(graph &g);
};

void match::init(graph &g) {
    for(VERTEX i = 0; i < g.adj.size(); i++) {
        if(g.adj[i].empty()) {
            status.push_back(state::done);
        }
        else {
            status.push_back(state::init);
        }
    }
    /*Note that non-negative weights are not allowed in this application*/
    for(VERTEX i = 0; i < g.adj.size(); i++) {
        storage.push_back(std::make_pair(0, -1));
    }
}

inline EDGE find_mate(const graph &g, VERTEX ll_u) {
    if(g.adj[ll_u].size() == 0) {
        return std::make_pair(0, -1.0);
    }
    return g.adj[ll_u].back();
}

class matchPkt {
    public:
        VERTEX u;
        VERTEX v;
        WEIGHT w;
        comm type;
        matchPkt(VERTEX _u, VERTEX _v, WEIGHT _w) {
            u = _u;
            v = _v;
            w = _w;
        }
        matchPkt() = default;

};

class MatchSelector: public hclib::Selector<3, matchPkt> {
    graph &g;
    std::vector<state> &status;
    std::vector<EDGE> &storage;
    std::vector<std::pair<VERTEX, VERTEX>> &final_set;

    void reject_storage(VERTEX ll_u, VERTEX u, int mb) {
        if(storage[ll_u].second != -1) {
            matchPkt rj(storage[ll_u].first, u, storage[ll_u].second);
            rj.type = comm::reject;
            send(mb, rj, g._owner(rj.u));
            storage[ll_u].second = -1;
        }
    }

    void inquiry(matchPkt pkt, int sender_rank) {
        if(pkt.type == comm::request) {
            //fprintf(stderr, "%ld, %ld\n", pkt.v, g._to_local(pkt.v));
            VERTEX ll_v = g._to_local(pkt.v);
            if(status[ll_v] == state::done) {
                pkt.type = comm::reject;
                send(1, pkt, g._owner(pkt.u));
            }
            else {
                EDGE mate_edge = find_mate(g, ll_v);
                if(mate_edge.second == -1) {
                    pkt.type = comm::reject;
                    send(1, pkt, g._owner(pkt.u));
                    reject_storage(ll_v, pkt.v, 1);
                    status[ll_v] = state::done;
                }
                else if(mate_edge.first == pkt.u) {
                    pkt.type = comm::accept;
                    send(1, pkt, g._owner(pkt.u));
                    if(storage[ll_v].first != pkt.u) {
                        reject_storage(ll_v, pkt.v, 1);
                    }
                    else {
                        storage[ll_v].second = -1;
                    }
                    status[ll_v] = state::done;
                }
                else {
                    if(storage[ll_v].second < pkt.w || (storage[ll_v].second == pkt.w && storage[ll_v].first > pkt.u)) {
                        reject_storage(ll_v, pkt.v, 1);
                        storage[ll_v].first = pkt.u;
                        storage[ll_v].second = pkt.w;
                    }
                    else {
                        pkt.type = comm::reject;
                        send(1, pkt, g._owner(pkt.u));
                    }
                }
            }
        }
        else if(pkt.type == comm::accept) {
            VERTEX ll_v = g._to_local(pkt.v);
            if(storage[ll_v].first != pkt.u) {
                reject_storage(ll_v, pkt.v, 1);
            }
            else {
                storage[ll_v].second = -1;
            }
            status[ll_v] = state::done;
            final_set.push_back(std::make_pair(pkt.u, pkt.v));
        }
    }
    void reply(matchPkt pkt, int sender_rank) {
        VERTEX ll_u = g._to_local(pkt.u);
        if(pkt.type == comm::accept) {
            status[ll_u] = state::done;
            if(storage[ll_u].first != pkt.v) {
                reject_storage(ll_u, pkt.u, 2);
            }
            else {
                storage[ll_u].second = -1;
            }
            final_set.push_back(std::make_pair(pkt.u, pkt.v));
        }
        else if(pkt.type == comm::reject) {
            status[ll_u] = state::init;
            assert(find_mate(g, ll_u).first == pkt.v);
            g.adj[ll_u].pop_back();
        }
    }

    void reject(matchPkt pkt, int sender_rank) {
        VERTEX ll_u = g._to_local(pkt.u);
        assert(pkt.type == comm::reject);
        status[ll_u] = state::init;
        assert(find_mate(g, ll_u).first == pkt.v);
        g.adj[ll_u].pop_back();
    }

public:
    MatchSelector(graph &_g, std::vector<state> &_status, std::vector<EDGE> &_storage, std::vector<std::pair<VERTEX, VERTEX>> &_final_set): 
        hclib::Selector<3, matchPkt>(true), g(_g), status(_status), storage(_storage), final_set(_final_set) {
        mb[0].process = [this](matchPkt pkt, int sender_rank) { this->inquiry(pkt, sender_rank); };
        mb[1].process = [this](matchPkt pkt, int sender_rank) { this->reply(pkt, sender_rank); };
        mb[2].process = [this](matchPkt pkt, int sender_rank) { this->reject(pkt, sender_rank); };
    }
};

void match::matching(graph &g) {
    double t1 = wall_seconds();
    while(1) {
        MatchSelector* matchSelector = new MatchSelector(g, status, storage, final_set);
        hclib::finish([&]() {
            for(VERTEX ll_u = 0; ll_u < g.adj.size(); ll_u++) {
                if(status[ll_u] == state::done || status[ll_u] == state::waiting) {
                    continue;
                }
                assert(status[ll_u] == state::init);
                EDGE mate_edge = find_mate(g, ll_u);
                assert(mate_edge.first < g.numberOfVertices);
                if(mate_edge.second == -1) {
                    status[ll_u] = state::done;
                    if(storage[ll_u].second != -1) {
                        matchPkt rj(storage[ll_u].first, g._to_global(ll_u), storage[ll_u].second);
                        rj.type = comm::reject;
                        matchSelector->send(1, rj, g._owner(rj.u));
                        storage[ll_u].second = -1;
                    }
                    continue;
                }
                matchPkt pkt(g._to_global(ll_u), mate_edge.first, mate_edge.second);
                /* if storage already has this vertice, send accept*/
                /* else send the request */
                if(storage[ll_u].second != -1) {
                    if(storage[ll_u].first == pkt.v) {
                        pkt.type = comm::accept;
                        matchSelector->send(0, pkt, g._owner(pkt.v));
                        status[ll_u] = state::done;
                        storage[ll_u].second = -1;
                        final_set.push_back(std::make_pair(pkt.u, pkt.v));
                        continue;
                    }
                }
                pkt.type = comm::request;
                status[ll_u] = state::waiting; 
                matchSelector->send(0, pkt, g._owner(pkt.v));
            }
            matchSelector->done(0);
        });
        delete matchSelector;
        int should_terminate = 0;
        for(auto x: status) {
            if(x != state::done) {
                should_terminate = 1;
                break;
            }
        }
        if(lgp_reduce_add_l(should_terminate) == 0) {
            break;
        }
        uint64_t total_size_matching = lgp_reduce_add_l(final_set.size());
        T0_fprintf(stderr, "Matching Size: %ld\n", total_size_matching/2);
    }
    T0_fprintf(stderr, "Time taken for matching: %.3f\n", wall_seconds() - t1);
    uint64_t total_size_matching = lgp_reduce_add_l(final_set.size());
    assert(total_size_matching % 2 == 0);
    T0_fprintf(stderr, "Matching Size: %ld\n", total_size_matching/2);
}