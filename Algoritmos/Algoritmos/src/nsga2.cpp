#include "nsga2.h"
#include <iostream>
#include <numeric>
#include <cmath>

NSGA2::NSGA2(const NSGA2Config& config)
    : cfg(config), rng(config.seed), n(0) {}

bool NSGA2::dominates(const Individual& a, const Individual& b) const {
    bool better_time   = (a.time   <= b.time);
    bool better_demand = (a.demand >= b.demand);
    bool strict_time   = (a.time   <  b.time);
    bool strict_demand = (a.demand >  b.demand);
    return (better_time && better_demand) && (strict_time || strict_demand);
}

void NSGA2::evaluate(const Graph& g, Individual& ind) {
    ind.time   = 0.0;
    ind.demand = 0.0;
    int prev = 0;
    for (int i = 0; i < ind.route_length; i++) {
        int v = ind.perm[i];
        double t = g.time_matrix[prev][v];
        if (t == INF) { ind.time = INF; return; }
        ind.time   += t;
        ind.demand += g.nodes[v].demand;
        prev = v;
    }
    double t_return = g.time_matrix[prev][0];
    if (t_return == INF) { ind.time = INF; return; }
    ind.time += t_return;
}

std::vector<Individual> NSGA2::init_population(const Graph& g) {
    std::vector<int> base(n);
    std::iota(base.begin(), base.end(), 1);
    std::vector<Individual> pop;
    pop.reserve(cfg.pop_size);
    std::uniform_int_distribution<int> dist_len(3, n);

    for (int i = 0; i < cfg.pop_size; i++) {
        Individual ind;
        ind.perm = base;
        std::shuffle(ind.perm.begin(), ind.perm.end(), rng);
        ind.route_length = dist_len(rng); // Tamanho aleatório inicial
        evaluate(g, ind);
        ind.rank = 0;
        ind.crowding = 0.0;
        pop.push_back(ind);
    }
    return pop;
}

std::vector<std::vector<int>> NSGA2::fast_nondominated_sort(const std::vector<Individual>& pop) {
    int M = pop.size();
    std::vector<int> domination_count(M, 0);
    std::vector<std::vector<int>> dominated_by(M);
    std::vector<std::vector<int>> fronts;
    fronts.push_back({});

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (i == j) continue;
            if (dominates(pop[i], pop[j])) {
                dominated_by[i].push_back(j);
            } else if (dominates(pop[j], pop[i])) {
                domination_count[i]++;
            }
        }
        if (domination_count[i] == 0) {
            fronts[0].push_back(i);
        }
    }

    int fi = 0;
    while (!fronts[fi].empty()) {
        std::vector<int> next_front;
        for (int i : fronts[fi]) {
            for (int j : dominated_by[i]) {
                domination_count[j]--;
                if (domination_count[j] == 0) {
                    next_front.push_back(j);
                }
            }
        }
        fi++;
        fronts.push_back(next_front);
    }
    fronts.pop_back();
    return fronts;
}

void NSGA2::crowding_distance(std::vector<Individual>& pop, const std::vector<int>& front) {
    int m = front.size();
    if (m <= 2) {
        for (int i : front) pop[i].crowding = 1e9;
        return;
    }
    for (int i : front) pop[i].crowding = 0.0;

    std::vector<int> sorted_front = front;
    std::sort(sorted_front.begin(), sorted_front.end(),
              [&](int a, int b) { return pop[a].time < pop[b].time; });
    pop[sorted_front.front()].crowding = 1e9;
    pop[sorted_front.back() ].crowding = 1e9;
    double time_range = pop[sorted_front.back()].time - pop[sorted_front.front()].time;
    if (time_range < 1e-9) time_range = 1.0;
    for (int i = 1; i < m - 1; i++) {
        pop[sorted_front[i]].crowding += (pop[sorted_front[i+1]].time - pop[sorted_front[i-1]].time) / time_range;
    }

    std::sort(sorted_front.begin(), sorted_front.end(),
              [&](int a, int b) { return pop[a].demand < pop[b].demand; });
    pop[sorted_front.front()].crowding = 1e9;
    pop[sorted_front.back() ].crowding = 1e9;
    double demand_range = pop[sorted_front.back()].demand - pop[sorted_front.front()].demand;
    if (demand_range < 1e-9) demand_range = 1.0;
    for (int i = 1; i < m - 1; i++) {
        pop[sorted_front[i]].crowding += (pop[sorted_front[i+1]].demand - pop[sorted_front[i-1]].demand) / demand_range;
    }
}

Individual NSGA2::tournament_select(const std::vector<Individual>& pop) {
    std::uniform_int_distribution<int> dist(0, pop.size() - 1);
    Individual best = pop[dist(rng)];
    for (int i = 1; i < cfg.tournament_k; i++) {
        const Individual& candidate = pop[dist(rng)];
        bool prefer = (candidate.rank < best.rank) ||
                      (candidate.rank == best.rank && candidate.crowding > best.crowding);
        if (prefer) best = candidate;
    }
    return best;
}

Individual NSGA2::ox_crossover(const Individual& p1, const Individual& p2) {
    int sz = p1.perm.size();
    std::uniform_int_distribution<int> dist(0, sz - 1);
    int a = dist(rng), b = dist(rng);
    if (a > b) std::swap(a, b);

    Individual child;
    child.perm.resize(sz, -1);

    for (int i = a; i <= b; i++) child.perm[i] = p1.perm[i];

    int ci = (b + 1) % sz;
    for (int i = 0; i < sz; i++) {
        int pi = (b + 1 + i) % sz;
        int gene = p2.perm[pi];
        bool found = false;
        for (int x = a; x <= b; x++) {
            if (child.perm[x] == gene) { found = true; break; }
        }
        if (!found) {
            child.perm[ci] = gene;
            ci = (ci + 1) % sz;
        }
    }
    
    // Herda o tamanho da rota aleatoriamente
    child.route_length = (std::uniform_int_distribution<int>(0, 1)(rng) == 0) ? p1.route_length : p2.route_length;
    child.rank = 0;
    child.crowding = 0.0;
    return child;
}

void NSGA2::mutate(Individual& ind) {
    int sz = ind.perm.size();
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    
    if (prob(rng) < 0.5) {
        std::uniform_int_distribution<int> dist(0, sz - 1);
        int a = dist(rng), b = dist(rng);
        if (a > b) std::swap(a, b);
        std::reverse(ind.perm.begin() + a, ind.perm.begin() + b + 1);
    } else {
        // Mutação no tamanho da rota
        ind.route_length += std::uniform_int_distribution<int>(-3, 3)(rng);
        ind.route_length = std::max(3, std::min(n, ind.route_length));
    }
}

ParetoFront NSGA2::solve(const Graph& g) {
    n = g.size() - 1;
    hv_history.clear();

    std::vector<Individual> pop = init_population(g);

    auto fronts = fast_nondominated_sort(pop);
    for (int fi = 0; fi < (int)fronts.size(); fi++)
        for (int i : fronts[fi]) pop[i].rank = fi;
    for (auto& f : fronts) crowding_distance(pop, f);

    std::uniform_real_distribution<double> prob(0.0, 1.0);

    for (int gen = 0; gen < cfg.generations; gen++) {
        std::vector<Individual> offspring;
        offspring.reserve(cfg.pop_size);

        while ((int)offspring.size() < cfg.pop_size) {
            Individual p1 = tournament_select(pop);
            Individual p2 = tournament_select(pop);

            Individual child;
            if (prob(rng) < cfg.crossover_rate) {
                child = ox_crossover(p1, p2);
            } else {
                child = (prob(rng) < 0.5) ? p1 : p2;
            }

            if (prob(rng) < cfg.mutation_rate) {
                mutate(child);
            }

            evaluate(g, child);
            if (child.time < INF) offspring.push_back(child);
        }

        std::vector<Individual> combined = pop;
        combined.insert(combined.end(), offspring.begin(), offspring.end());

        auto all_fronts = fast_nondominated_sort(combined);
        for (int fi = 0; fi < (int)all_fronts.size(); fi++)
            for (int i : all_fronts[fi]) combined[i].rank = fi;
        for (auto& f : all_fronts) crowding_distance(combined, f);

        pop.clear();
        for (auto& front : all_fronts) {
            if ((int)pop.size() + (int)front.size() <= cfg.pop_size) {
                for (int i : front) pop.push_back(combined[i]);
            } else {
                std::sort(front.begin(), front.end(),
                          [&](int a, int b) {
                              return combined[a].crowding > combined[b].crowding;
                          });
                int remaining = cfg.pop_size - pop.size();
                for (int i = 0; i < remaining; i++)
                    pop.push_back(combined[front[i]]);
                break;
            }
            if ((int)pop.size() >= cfg.pop_size) break;
        }

        ParetoFront pf_gen;
        for (const auto& ind : pop) {
            if (ind.rank == 0) {
                ParetoPoint pp;
                pp.route  = ind.perm;
                pp.time   = ind.time;
                pp.demand = ind.demand;
                pp.alpha  = 0;
                pp.beta   = 0;
                pf_gen.add(pp);
            }
        }
        hv_history.push_back(pf_gen.hypervolume());
    }

    ParetoFront result;
    for (const auto& ind : pop) {
        if (ind.rank == 0) {
            ParetoPoint pp;
            pp.route.push_back(0);
            for (int i = 0; i < ind.route_length; i++) pp.route.push_back(ind.perm[i]);
            pp.route.push_back(0);
            pp.time   = ind.time;
            pp.demand = ind.demand;
            pp.alpha  = 0;
            pp.beta   = 0;
            result.add(pp);
        }
    }
    return result;
}
