#ifndef EXACT_H
#define EXACT_H
#include "graph.h"

struct ExactSolution {
    std::vector<int> route;
    double total_time, total_demand;
    long long nodes_explored;
};

class BranchAndBound {
    const Graph* graph;
    int n;
    double best_cost;
    ExactSolution best_sol;
    std::vector<bool> visited;
    std::vector<int> current_path;
    long long nodes_explored;

public:
    ExactSolution solve(const Graph& g, double alpha, double beta);
private:
    double lower_bound(double current_cost, int depth, double alpha, double beta);
    void branch(int current, double current_cost, int depth, double alpha, double beta, double current_demand);
};
#endif