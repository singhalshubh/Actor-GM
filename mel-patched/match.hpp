
/* Maximal edge matching using MPI RMA */

#pragma once
#ifndef MAXEMATCHRMA_HPP
#define MAXEMATCHRMA_HPP

#include "graph.hpp"

#include <numeric>
#include <utility>
#include <cstring>
#include <cassert>

#define MATE_REQUEST 1
#define MATE_REJECT 2 
#define MATE_ACCEPT 3
#define MATE_ACCEPT_D 4

enum class state {
    init = 0,
    waiting = 1,
    done = 2
};

// TODO FIXME change comm_world to comm_

class MaxEdgeMatchRMAFix
{
    public:
        MaxEdgeMatchRMAFix(Graph* g): 
            g_(g), D_(0), M_(0), status(nullptr),
            qbuf_(nullptr), ghost_count_(nullptr),
            nghosts_(0), nghosts_indices_(0), rdispls_(0),
            scounts_(0), rcounts_(0), prcounts_(0),
            storage_vertex(nullptr), storage_weight(nullptr),
            win_(MPI_WIN_NULL), wbuf_(nullptr)
        {
            MPI_Comm_size(MPI_COMM_WORLD, &size_);
            MPI_Comm_rank(MPI_COMM_WORLD, &rank_);

            const GraphElem lnv = g_->get_lnv();
            status = new state[lnv];
            storage_vertex = new GraphElem[lnv];
            storage_weight = new GraphWeight[lnv];
            ghost_count_ = new GraphElem[lnv];
            
            std::fill(status, status + lnv, state::init);
            std::fill(storage_vertex, storage_vertex + lnv, -1);
            std::fill(storage_weight, storage_weight + lnv, 0.0);
            std::fill(ghost_count_, ghost_count_ + lnv, 0);

            rdispls_.resize(size_, 0);
            nghosts_.resize(size_, 0);
            nghosts_indices_.resize(size_, 0);
            
            GraphElem tot_ghosts = 0;

            for (GraphElem i = 0; i < lnv; i++)
            {
                GraphElem e0, e1;
                g_->edge_range(i, e0, e1);

                for (GraphElem e = e0; e < e1; e++)
                {
                    Edge const& edge = g_->get_edge(e);
                    const int p = g_->get_owner(edge.tail_);
                    if (p != rank_)
                    {
                        nghosts_[p] += 1;
                        ghost_count_[i] += 1;
                    }
                }

                tot_ghosts += ghost_count_[i];
            }

            nelems_ = tot_ghosts*2*3;
            qbuf_ = new GraphElem[nelems_];
            GraphElem disp = 0;
            for (int p = 0; p < size_; p++)
            {
                nghosts_indices_[p] = disp;
                disp += nghosts_[p]*2*3;
            }
            MPI_Alltoall(nghosts_indices_.data(), 1, MPI_GRAPH_TYPE, 
                    rdispls_.data(), 1, MPI_GRAPH_TYPE, MPI_COMM_WORLD);
            
            scounts_.resize(size_, 0);
            rcounts_.resize(size_, 0);
            prcounts_.resize(size_, 0);
        }
        
        void create_mpi_win()
        {
            MPI_Win_allocate(nelems_*sizeof(GraphElem), sizeof(GraphElem), 
                    MPI_INFO_NULL, MPI_COMM_WORLD, &wbuf_, &win_);             
            MPI_Win_lock_all(MPI_MODE_NOCHECK, win_);
        }

        void destroy_mpi_win() 
        {             
            MPI_Win_unlock_all(win_);
            MPI_Win_free(&win_); 
        }

        ~MaxEdgeMatchRMAFix() {}

        void clear()
        {
            M_.clear();
            D_.clear();
            
            delete []qbuf_;
            delete []ghost_count_;
            delete []status;
            delete []storage_vertex;
            delete []storage_weight;

            rdispls_.clear();
            rcounts_.clear();
            prcounts_.clear();
            scounts_.clear();
            nghosts_.clear();
            nghosts_indices_.clear();
        }
       
        inline bool is_same(GraphWeight a, GraphWeight b) 
        { return std::abs(a - b) <= std::numeric_limits<GraphWeight>::epsilon(); }

        struct DirRec {
            GraphElem a, b;
            unsigned char dir;
        };

        void print_M() const
        {
            // gather M_
            unsigned int m_size = M_.size(), m_global_size = 0;
            // i,j
            m_size *= 2;
            GraphElem* M_buf = new GraphElem[m_size];

            GraphElem* M_global = nullptr;
            int* rcounts = nullptr;
            int* rdispls = nullptr;

            // communication params
            if (rank_ == 0)
            {
                rcounts = new int[size_];
                rdispls = new int[size_];
            }

            // put M_ into a contiguous buffer
            for (int i = 0, j = 0; i < m_size; i+=2, j++)
            {
                M_buf[i]    = M_[j].ij_[0];
                M_buf[i+1]  = M_[j].ij_[1];
            }

            MPI_Gather(&m_size, 1, MPI_INT, rcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Reduce(&m_size, &m_global_size, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

            // communication params (at root)
            if (rank_ == 0)
            {
                M_global = new GraphElem[m_global_size];

                unsigned int index = 0;
                for (int p = 0; p < size_; p++)
                {
                    rdispls[p] = index;
                    index += rcounts[p];
                }
            }

            MPI_Gatherv(M_buf, m_size, MPI_GRAPH_TYPE, M_global, rcounts, rdispls, 
                    MPI_GRAPH_TYPE, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);

            // print mates
            if (rank_ == 0)
            {
                std::vector<std::pair<GraphElem,GraphElem>> edges;
                edges.reserve(m_global_size);

                for (int i = 0; i + 1 < m_global_size; i += 2) {
                    GraphElem u = M_global[i];
                    GraphElem v = M_global[i+1];
                    if (u == (GraphElem)-1 || v == (GraphElem)-1 || u == v) continue;
                    if (u > v) std::swap(u, v);
                    edges.emplace_back(u, v);
                }

                // dedup globally
                std::sort(edges.begin(), edges.end());
                edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

                std::cout << "Unique matchings: " << edges.size() << "\n";
            }
            
            MPI_Barrier(MPI_COMM_WORLD);

            // clear buffers
            delete []M_global;
            delete []M_buf;
            delete []rcounts;
            delete []rdispls;
        }

        std::vector<EdgeTuple> const& operator()()
        {
            maxematch_rma();
            print_M();
            return M_;
        }

        // initiate put
        void Put(int tag, int target, GraphElem data[2])
        {
            assert(data[0] != (GraphElem)-1 && data[1] != (GraphElem)-1);
            assert(target >= 0 && target < size_);
            const GraphElem cap = nghosts_[target]*2*3;
            assert(scounts_[target] + 3 <= cap);
            const GraphElem index = nghosts_indices_[target] + scounts_[target];

            qbuf_[index] = data[0];
            qbuf_[index + 1] = data[1];
            qbuf_[index + 2] = tag;
            GraphElem tdisp = rdispls_[target] + scounts_[target];
            MPI_Put(&qbuf_[index], 3, MPI_GRAPH_TYPE, target, 
                    (MPI_Aint)tdisp, 3, MPI_GRAPH_TYPE, win_);
            scounts_[target] += 3;
        }

        void compute_mate(const GraphElem ll_v, Edge& max_edge) {
            GraphElem e0, e1;
            g_->edge_range(ll_v, e0, e1);
            for (GraphElem e = e0; e < e1; e++) {
                EdgeActive& edge = g_->get_active_edge(e);
                if (edge.active_) {
                    if (edge.edge_.weight_ > max_edge.weight_)
                        max_edge = edge.edge_;

                    if (is_same(edge.edge_.weight_, max_edge.weight_))
                        if (edge.edge_.tail_ > max_edge.tail_)
                            max_edge = edge.edge_;
                }
            }
        }

        inline void deactivate_edge(GraphElem x, GraphElem y) {
            GraphElem e0, e1;
            const GraphElem lx = g_->global_to_local(x);
            const int y_owner = g_->get_owner(y);
            g_->edge_range(lx, e0, e1);
            for (GraphElem e = e0; e < e1; e++) {
                EdgeActive& edge = g_->get_active_edge(e);
                if (edge.edge_.tail_ == y && edge.active_) {
                    edge.active_ = false;
                    break;
                }
            }
        }

        GraphWeight find_weight(GraphElem v, GraphElem u) {
            GraphElem ll_v = g_->global_to_local(v);
            GraphElem e0, e1;
            g_->edge_range(ll_v, e0, e1);

            for (GraphElem e = e0; e < e1; e++)
            {
                Edge const& edge = g_->get_edge(e);
                if(edge.tail_ == u) {
                    return edge.weight_;
                }
            }
            assert(0);
            return -1;
        }

        void process_window(int mode)
        {
            MPI_Win_flush_all(win_);
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Alltoall(scounts_.data(), 1, MPI_GRAPH_TYPE, 
                    rcounts_.data(), 1, MPI_GRAPH_TYPE, MPI_COMM_WORLD);
            
            
            for (int k = 0; k < size_; k++) {
                const GraphElem index = nghosts_indices_[k];
                const GraphElem start = prcounts_[k];
                const GraphElem end = rcounts_[k];
                
                for (int i = start; i < end; i+=3) {
                    if(mode == 0) {
                        pull_mb0(wbuf_[index+i], wbuf_[index+i+1], wbuf_[index+i+2]);
                    }
                    else if(mode == 1) {
                        if(wbuf_[index+i+2] == MATE_ACCEPT) {
                            pull_accept(wbuf_[index+i], wbuf_[index+i+1], wbuf_[index+i+2]);
                        }
                    }
                    else if(mode == 2) {
                        if(wbuf_[index+i+2] == MATE_REJECT) {
                            pull_reject(wbuf_[index+i], wbuf_[index+i+1], wbuf_[index+i+2]);
                        }
                    }
                }
                if(mode == 2) {
                    prcounts_[k] = rcounts_[k];
                    rcounts_[k] = 0;
                }
            }
        }

        void pull_mb0(GraphElem u, GraphElem v, int comm) {
            assert(g_->get_owner(u) < size_);
            GraphElem pkt[2] = {u, v};
            if(comm == MATE_REQUEST) {
                GraphElem ll_v = g_->global_to_local(v);
                if(status[ll_v] == state::done) {
                    if(g_->get_owner(u) == g_->get_owner(v)) {
                        pull_reject(u, v, MATE_REJECT);
                    }
                    else {
                        Put(MATE_REJECT, g_->get_owner(u), pkt);
                    } 
                }
                else {
                    Edge mate_edge;
                    compute_mate(ll_v, mate_edge);
                    if(mate_edge.tail_ == (GraphElem)-1) {
                        if(g_->get_owner(u) == g_->get_owner(v)) {
                            pull_reject(u, v, MATE_REJECT);
                        }
                        else {
                            Put(MATE_REJECT, g_->get_owner(u), pkt); 
                        }
                        reject_storage(ll_v);
                        status[ll_v] = state::done;
                    }
                    else if(mate_edge.tail_ == u) {
                        if(g_->get_owner(u) == g_->get_owner(v)) {
                            pull_accept(u, v, MATE_ACCEPT);
                        }
                        else {
                            Put(MATE_ACCEPT, g_->get_owner(u), pkt); 
                        }
                        if(storage_vertex[ll_v] != u) {
                            reject_storage(ll_v);
                        }
                        else {
                            storage_vertex[ll_v] = (GraphElem)-1;
                        }
                        status[ll_v] = state::done;
                    }
                    else {
                        GraphWeight w = find_weight(v, u);
                        if(storage_weight[ll_v] < w || (is_same(storage_weight[ll_v], w) && storage_vertex[ll_v] > u)) {
                            reject_storage(ll_v);
                            storage_vertex[ll_v] = u;
                            storage_weight[ll_v] = w;
                        }
                        else {
                            if(g_->get_owner(u) == g_->get_owner(v)) {
                                pull_reject(u, v, MATE_REJECT);
                            }
                            else {
                                Put(MATE_REJECT, g_->get_owner(u), pkt);
                            } 
                        }
                    }
                }
            }
            else if(comm == MATE_ACCEPT_D) {
                GraphElem ll_v = g_->global_to_local(v);
                if(storage_vertex[ll_v] != u) {
                    reject_storage(ll_v);
                }
                else {
                    storage_vertex[ll_v] = (GraphElem)-1;
                }
                status[ll_v] = state::done;
                M_.emplace_back(u, v, 0.0);
            }
        }

        void reject_storage(GraphElem ll_u) {
            if(storage_vertex[ll_u] != (GraphElem)-1) {
                GraphElem u = g_->local_to_global(ll_u);
                assert(g_->get_owner(storage_vertex[ll_u]) < size_);
                GraphElem pkt[2] = {storage_vertex[ll_u], u};
                if(g_->get_owner(storage_vertex[ll_u]) != g_->get_owner(u)) {
                    Put(MATE_REJECT, g_->get_owner(storage_vertex[ll_u]), pkt); 
                }
                else {
                    pull_reject(storage_vertex[ll_u], u, MATE_REJECT);
                }
                storage_vertex[ll_u] = (GraphElem)-1;
            }
        }

        void pull_accept(GraphElem u, GraphElem v, int comm) {
            assert(comm == MATE_ACCEPT);
            GraphElem ll_u = g_->global_to_local(u);
            status[ll_u] = state::done;
            if(storage_vertex[ll_u] != v) {
                reject_storage(ll_u);
            }
            else {
                storage_vertex[ll_u] = (GraphElem)-1;
            }
            M_.emplace_back(u, v, 0.0);
        }

        void pull_reject(GraphElem u, GraphElem v, int comm) {
            assert(comm == MATE_REJECT);
            GraphElem ll_u = g_->global_to_local(u);
            if (status[ll_u] == state::done) return;
            status[ll_u] = state::init;
            deactivate_edge(u, v);
        }

        void maxematch_rma()
        {
            while(1) {
                for (GraphElem ll_u = 0; ll_u < g_->get_lnv(); ll_u++) {
                    GraphElem u = g_->local_to_global(ll_u);
                    if(status[ll_u] == state::done || status[ll_u] == state::waiting) {
                        continue;
                    }
                    assert(status[ll_u] == state::init);
                    Edge mate_edge;
                    compute_mate(ll_u, mate_edge); 
                    // std::cout << g_->local_to_global(ll_u) << "-->" << mate_edge.tail_ << std::endl;
                    if(mate_edge.tail_ == (GraphElem)-1) {
                        status[ll_u] = state::done;
                        reject_storage(ll_u);
                        continue;
                    }
                    GraphElem pkt[2] = {u, mate_edge.tail_};
                    if(storage_vertex[ll_u] != (GraphElem)-1) {
                        if(storage_vertex[ll_u] == mate_edge.tail_) {
                            if(g_->get_owner(mate_edge.tail_) == g_->get_owner(u)) {
                                pull_mb0(u, mate_edge.tail_, MATE_ACCEPT_D);
                            }
                            else {
                                Put(MATE_ACCEPT_D, g_->get_owner(mate_edge.tail_), pkt); 
                            }
                            status[ll_u] = state::done;
                            storage_vertex[ll_u] = (GraphElem)-1;
                            M_.emplace_back(u, mate_edge.tail_, 0.0);
                            continue;
                        }
                    }
                    status[ll_u] = state::waiting; 
                    if(g_->get_owner(mate_edge.tail_) == g_->get_owner(u)) {
                        pull_mb0(u, mate_edge.tail_, MATE_REQUEST);
                    }
                    else {
                        Put(MATE_REQUEST, g_->get_owner(mate_edge.tail_), pkt); 
                    }
                }
                process_window(0);
                process_window(1);
                process_window(2);
                MPI_Barrier(MPI_COMM_WORLD);   
                int count = 0;
                for(GraphElem ll_u = 0; ll_u < g_->get_lnv(); ll_u++) {
                    if(status[ll_u] != state::done) {
                        count = 1;
                        break;
                    }
                }    
                MPI_Allreduce(MPI_IN_PLACE, &count, 1, MPI_GRAPH_TYPE, 
                        MPI_SUM, MPI_COMM_WORLD);
                if(rank_ == 0) std::cout << count << std::endl;
                if (count == 0)
                    break; 
            }
        } 

    private:
        Graph* g_;
        std::vector<GraphElem> D_;
        std::vector<EdgeTuple> M_;
        state *status;
        GraphElem *storage_vertex;
        GraphWeight *storage_weight;
        GraphElem nelems_;
        GraphElem *ghost_count_; 
        GraphElem *qbuf_; 
        GraphElem *countp_;
        std::vector<GraphElem> nghosts_, nghosts_indices_, rdispls_;
        std::vector<GraphElem> scounts_, rcounts_, prcounts_;
        GraphElem *wbuf_;
        MPI_Win win_;
        int rank_, size_;
};

#endif