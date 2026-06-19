#ifndef NSGA2_H
#define NSGA2_H
#include "graph.h"
#include "pareto.h"
#include <vector>
#include <random>
#include <algorithm>

struct NSGA2Config {
    int pop_size = 50;
    int generations = 100;
    double crossover_rate = 0.9;
    double mutation_rate = 0.1;
    int tournament_k = 2;
    int seed = 42;
};

struct Individual {
    std::vector<int> perm;
    int route_length; // NOVO: permite rotas de tamanhos diferentes
    double time, demand;
    int rank;
    double crowding;
};

class NSGA2 {
    NSGA2Config cfg;
    std::mt19937 rng;
    int n;

public:
    std::vector<double> hv_history;
    NSGA2(const NSGA2Config& config);
    ParetoFront solve(const Graph& g);

private:
    bool dominates(const Individual& a, const Individual& b) const;
    void evaluate(const Graph& g, Individual& ind);
    std::vector<Individual> init_population(const Graph& g);
    std::vector<std::vector<int>> fast_nondominated_sort(const std::vector<Individual>& pop);
    void crowding_distance(std::vector<Individual>& pop, const std::vector<int>& front);
    Individual tournament_select(const std::vector<Individual>& pop);
    Individual ox_crossover(const Individual& p1, const Individual& p2);
    void mutate(Individual& ind);
};
#endif