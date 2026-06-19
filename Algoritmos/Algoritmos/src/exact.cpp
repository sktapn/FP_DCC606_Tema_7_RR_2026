#include "exact.h"
#include <iostream>

/*
 * exact.cpp - Implementacao Branch-and-Bound para TSP
 *
 * Estrategia de poda:
 *   Se lower_bound(estado_atual) >= melhor_custo_conhecido -> poda
 *
 * Lower bound simplificado: para cada vertice nao visitado,
 * soma o menor arco disponivel. Divide por 2 (cada arco e contado 2x).
 * Nao e admissivel no sentido estrito do TSP asimetrico, mas funciona
 * como heuristica de poda eficaz.
 */

ExactSolution BranchAndBound::solve(const Graph& g, double alpha, double beta) {
    n = g.size();
    graph = &g;
    best_cost = INF;
    nodes_explored = 0;

    best_sol.route.clear();
    best_sol.total_time = INF;
    best_sol.total_demand = 0;
    best_sol.nodes_explored = 0;

    visited.assign(n, false);
    current_path.clear();

    // Comecar sempre pelo vertice 0 (Terminal Central / garagem)
    visited[0] = true;
    current_path.push_back(0);

    branch(0, 0.0, 1, alpha, beta, 0.0);

    best_sol.nodes_explored = nodes_explored;
    return best_sol;
}

double BranchAndBound::lower_bound(double current_cost, int /*depth*/, double alpha, double /*beta*/) {
    // Para vertices ainda nao visitados, estimar o menor custo possivel
    // Complexidade: O(n^2)
    double lb = current_cost;
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            double min_arc = INF;
            for (int j = 0; j < n; j++) {
                if (i != j && graph->time_matrix[i][j] < min_arc) {
                    min_arc = graph->time_matrix[i][j];
                }
            }
            if (min_arc < INF) lb += alpha * min_arc;
        }
    }
    return lb;
}

void BranchAndBound::branch(int current, double current_cost, int depth,
                             double alpha, double beta,
                             double current_demand) {
    nodes_explored++;

    // Todos os vertices visitados: fechar o circuito voltando ao vertice 0
    if (depth == n) {
        double return_time = graph->time_matrix[current][0];
        if (return_time == INF) return; // Nao ha arco de retorno

        double total_time   = current_cost + return_time;
        double total_demand = current_demand;

        // Custo escalarizado: minimizar alpha*tempo - beta*demanda
        double cost = alpha * total_time - beta * total_demand;

        if (cost < best_cost) {
            best_cost = cost;
            best_sol.route = current_path;
            best_sol.route.push_back(0); // Fecha o circuito
            best_sol.total_time   = total_time;
            best_sol.total_demand = total_demand;
        }
        return;
    }

    // Poda: se o lower bound ja supera o melhor custo, descartar ramo
    double lb = lower_bound(current_cost, depth, alpha, beta);
    if (lb >= best_cost) return;

    // Expandir cada vizinho nao visitado
    for (int next = 1; next < n; next++) { // 0 e a garagem, volta no fim
        if (!visited[next] && graph->time_matrix[current][next] < INF) {
            visited[next] = true;
            current_path.push_back(next);

            double arc_time   = graph->time_matrix[current][next];
            double arc_demand = graph->nodes[next].demand;

            // Custo parcial escalarizado: minimizar alpha*tempo, SEPARADO da demanda
            // Usamos apenas o tempo como custo de busca; demanda e tratada na solucao final
            double new_cost = current_cost + alpha * arc_time;

            branch(next, new_cost, depth + 1, alpha, beta,
                   current_demand + arc_demand);

            // Backtrack
            visited[next] = false;
            current_path.pop_back();
        }
    }
}
