#ifndef GREEDY_H
#define GREEDY_H
#include "graph.h"

struct GreedySolution {
    std::vector<int> route;
    double total_time, total_demand;
};

class GreedySolver {
public:
    GreedySolution solve(const Graph& g, double alpha, double beta, bool apply2opt = true);
private:
    GreedySolution construct(const Graph& g, double alpha, double beta);
    void two_opt(const Graph& g, GreedySolution& sol);
    double calc_time(const Graph& g, const std::vector<int>& route);
    double calc_demand(const Graph& g, const std::vector<int>& route);
};
#endif