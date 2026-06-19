#include "greedy.h"
#include <algorithm>
#include <iostream>

double GreedySolver::calc_time(const Graph& g, const std::vector<int>& route) {
    double total = 0.0;
    int sz = route.size();
    for (int i = 0; i < sz - 1; i++) {
        double t = g.time_matrix[route[i]][route[i+1]];
        if (t == INF) return INF;
        total += t;
    }
    return total;
}

double GreedySolver::calc_demand(const Graph& g, const std::vector<int>& route) {
    double total = 0.0;
    for (int i = 1; i < (int)route.size() - 1; i++) {
        total += g.nodes[route[i]].demand;
    }
    return total;
}

GreedySolution GreedySolver::construct(const Graph& g, double alpha, double beta) {
    int n = g.size();
    std::vector<bool> visited(n, false);
    GreedySolution sol;
    sol.route.push_back(0);
    visited[0] = true;
    int current = 0;

    for (int step = 1; step < n; step++) {
        double best_utility = -INF;
        int    best_next    = -1;

        for (int j = 1; j < n; j++) {
            if (!visited[j] && g.time_matrix[current][j] < INF) {
                double u = beta * g.nodes[j].demand - alpha * g.time_matrix[current][j];
                if (u > best_utility) {
                    best_utility = u;
                    best_next    = j;
                }
            }
        }

        if (best_next == -1) break;

        // PULO DO GATO: Parar a rota se a utilidade for negativa e já tivermos o mínimo de 4 paradas!
        // Cria a variação de rotas parciais (Trade-off Tempo vs Demanda)
        if (best_utility < 0 && step >= 4) {
            break;
        }

        visited[best_next] = true;
        sol.route.push_back(best_next);
        current = best_next;
    }

    sol.route.push_back(0);
    sol.total_time   = calc_time(g, sol.route);
    sol.total_demand = calc_demand(g, sol.route);
    return sol;
}

void GreedySolver::two_opt(const Graph& g, GreedySolution& sol) {
    bool improved = true;
    int sz = sol.route.size();
    while (improved) {
        improved = false;
        for (int i = 0; i < sz - 2; i++) {
            for (int k = i + 2; k < sz - 1; k++) {
                double d1 = g.time_matrix[sol.route[i  ]][sol.route[i+1]];
                double d2 = g.time_matrix[sol.route[k  ]][sol.route[k+1]];
                double d3 = g.time_matrix[sol.route[i  ]][sol.route[k  ]];
                double d4 = g.time_matrix[sol.route[i+1]][sol.route[k+1]];

                if (d1 == INF || d2 == INF) continue;
                if (d3 == INF || d4 == INF) continue;

                if (d3 + d4 < d1 + d2) {
                    std::reverse(sol.route.begin() + i + 1, sol.route.begin() + k + 1);
                    improved = true;
                }
            }
        }
    }
    sol.total_time   = calc_time(g, sol.route);
    sol.total_demand = calc_demand(g, sol.route);
}

GreedySolution GreedySolver::solve(const Graph& g, double alpha, double beta, bool apply2opt) {
    GreedySolution sol = construct(g, alpha, beta);
    if (apply2opt && sol.total_time < INF) {
        two_opt(g, sol);
    }
    return sol;
}
