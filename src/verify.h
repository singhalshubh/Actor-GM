/*
    for any edge selected in matching u--v
    - delete adj[u] and adj[v]
    - delete w--u or w--v
*/

class VerifySelector: public hclib::Selector<1, VERTEX> {
    graph &g;
    std::vector<std::pair<VERTEX, VERTEX>> &final_set;

    void process(VERTEX u, int sender_rank) {
        VERTEX ll_u = g._to_local(u);
        if(g._owner(u) == MYTHREAD) {
            g.adj[ll_u].clear();
        }
        for(auto &el: g.adj) {
            el.erase(std::remove_if(el.begin(), el.end(),
                        [u](const EDGE &p) { return p.first == u; }),
                     el.end());
        }
    }

public:
    VerifySelector(graph &_g, std::vector<std::pair<VERTEX, VERTEX>> &_final_set): 
        hclib::Selector<1, VERTEX>(true), g(_g), final_set(_final_set) {
        mb[0].process = [this](VERTEX pkt, int sender_rank) { this->process(pkt, sender_rank); };
    }
};

void verify(graph &g, std::vector<std::pair<VERTEX, VERTEX>> &final_set) {
    double t1 = wall_seconds();
    VerifySelector* verifySelector = new VerifySelector(g, final_set);
    hclib::finish([&]() {
        for(auto edge: final_set) {
            for(uint64_t pe = 0; pe < THREADS; pe++) {
                verifySelector->send(0, edge.first, pe);
                verifySelector->send(0, edge.second, pe);
            }
        }
        verifySelector->done(0);
    });
    delete verifySelector;
    /*
        After deletion, there should exist no adj[u] which contains an edge
    */
    for(auto &el: g.adj) {
        assert(el.size() == 0);
    }
    T0_fprintf(stderr, "Time taken for verification: %.3f\n", wall_seconds() - t1);
}