#include "graph.h"

/*
 * graph.cpp - Implementacao do grafo dirigido ponderado
 * Complexidade: construtor O(n^2), load O(n^2)
 */

Graph::Graph(int n) : n(n) {
    // Inicializa vertices com ids sequenciais
    nodes.resize(n);
    for (int i = 0; i < n; i++) {
        nodes[i].id = i;
        nodes[i].demand = 0.0;
        nodes[i].lat = 0.0;
        nodes[i].lon = 0.0;
    }
    // Matriz de tempos: O(n^2) espaco, inicializada com INF
    time_matrix.assign(n, std::vector<double>(n, INF));
    // Diagonal principal = 0 (sem custo de sair de si mesmo)
    for (int i = 0; i < n; i++) time_matrix[i][i] = 0.0;
}

void Graph::set_demand(int id, double demand) {
    if (id >= 0 && id < n)
        nodes[id].demand = demand;
}

void Graph::set_time(int from, int to, double time) {
    if (from >= 0 && from < n && to >= 0 && to < n)
        time_matrix[from][to] = time;
}

bool Graph::load_from_file(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Erro: nao foi possivel abrir " << filename << std::endl;
        return false;
    }

    // Primeira linha: numero de vertices
    f >> n;
    nodes.resize(n);
    time_matrix.assign(n, std::vector<double>(n, INF));
    for (int i = 0; i < n; i++) time_matrix[i][i] = 0.0;

    // Proximas n linhas: id nome demanda lat lon
    for (int i = 0; i < n; i++) {
        f >> nodes[i].id >> nodes[i].name >> nodes[i].demand
          >> nodes[i].lat >> nodes[i].lon;
    }

    // Arcos: from to time (ate o fim do arquivo)
    int from, to;
    double t;
    while (f >> from >> to >> t) {
        set_time(from, to, t);
    }

    f.close();
    return true;
}

void Graph::print_time_matrix() const {
    std::cout << "Matriz de Tempos (tij) [" << n << "x" << n << "]:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (time_matrix[i][j] == INF)
                std::cout << "  INF";
            else
                std::cout << "  " << time_matrix[i][j];
        }
        std::cout << "\n";
    }
}
