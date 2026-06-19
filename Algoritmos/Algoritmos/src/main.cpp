/*
 * main.cpp - Motor de Otimizacao de Rotas de Transporte Publico
 * Municipio de Boa Vista - RR
 * DCC606 - Analise de Algoritmos - UFRR 2026
 *
 * Executa:
 *   1. Algoritmo exato (B&B) em instancia pequena
 *   2. Heuristica gulosa (Vizinho Mais Proximo + 2-opt) na instancia Boa Vista
 *   3. NSGA-II multiobjetivo na instancia Boa Vista
 *   4. Exporta fronteira de Pareto para CSV
 *   5. Imprime tabela de resultados experimentais
 */

#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include "graph.h"
#include "exact.h"
#include "greedy.h"
#include "nsga2.h"
#include "pareto.h"

// Mede tempo de execucao em milissegundos
using Clock = std::chrono::high_resolution_clock;
using MS    = std::chrono::duration<double, std::milli>;

// Imprime uma linha de separacao
void sep() { std::cout << std::string(60, '-') << "\n"; }

// ----------------------------------------------------------------
// Cria a instancia de Boa Vista com 30 nos (hardcoded para garantir
// que o projeto funciona mesmo sem o arquivo de entrada)
// ----------------------------------------------------------------
Graph create_boa_vista() {
    // 30 nos: 0 = Terminal Central (garagem), 1-29 = pontos de parada
    const int N = 30;
    Graph g(N);

    // Nomes e demandas dos nos (passageiros por hora de pico)
    struct NodeData { const char* name; double demand; double lat; double lon; };
    NodeData nodes[] = {
        {"Terminal Central",          0,   2.8235, -60.6758},
        {"UFRR Paricarana",         300,   2.8503, -60.7169},
        {"Hospital Geral RR (HGR)", 280,   2.8197, -60.6892},
        {"Shopping Canarinho",      250,   2.8341, -60.6631},
        {"Mercado Municipal",       220,   2.8218, -60.6736},
        {"Aeroporto Atlas",         200,   2.8558, -60.6902},
        {"IFRR Campus BV",          150,   2.8087, -60.6801},
        {"Bairro Caranã",           180,   2.7956, -60.6998},
        {"Bairro Centenário",       170,   2.8421, -60.6893},
        {"Bairro Alvorada",         140,   2.8112, -60.7231},
        {"Bairro Nasa",             130,   2.7834, -60.7012},
        {"Bairro Pintolândia",      125,   2.7712, -60.6934},
        {"Bairro Cauamé",           120,   2.8634, -60.7234},
        {"Bairro Calungá",          115,   2.8156, -60.6612},
        {"Bairro Jóquei",           110,   2.8278, -60.6583},
        {"Bairro Liberdade",        105,   2.8089, -60.6657},
        {"Bairro São Bento",        100,   2.8367, -60.7023},
        {"Bairro São Francisco",     95,   2.8445, -60.7145},
        {"Bairro Jardim Floresta",   90,   2.7923, -60.6756},
        {"Bairro Senador H. Castro", 85,   2.8189, -60.7345},
        {"Bairro Raiar do Sol",      80,   2.7867, -60.7189},
        {"Bairro Dr. Silvio Leite",  75,   2.8534, -60.6712},
        {"Bairro Tancredo Neves",    70,   2.8023, -60.6523},
        {"Distrito Industrial",     160,   2.9012, -60.6845},
        {"Zona Sul - Novo Paraíso",  65,   2.7634, -60.7034},
        {"Zona Oeste - Pricumã",     60,   2.8712, -60.7523},
        {"CERR (Companhia Energia)", 55,   2.8289, -60.6489},
        {"Praça do Centro Cívico",  145,   2.8234, -60.6723},
        {"Bairro União",             50,   2.8501, -60.6612},
        {"Bairro Monte Sinai",       45,   2.7789, -60.6867}
    };

    for (int i = 0; i < N; i++) {
        g.nodes[i].name   = nodes[i].name;
        g.nodes[i].demand = nodes[i].demand;
        g.nodes[i].lat    = nodes[i].lat;
        g.nodes[i].lon    = nodes[i].lon;
    }

    // Gerar matriz de tempos usando distancia euclidiana aproximada
    // 1 grau ~ 111 km; velocidade media urbana = 30 km/h -> 1 grau ~ 222 min
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) { g.time_matrix[i][j] = 0.0; continue; }
            double dlat = nodes[i].lat - nodes[j].lat;
            double dlon = nodes[i].lon - nodes[j].lon;
            double dist_graus = std::sqrt(dlat*dlat + dlon*dlon);
            // Converte para minutos: dist_km * (60/30 km/h) = dist_km * 2
            double dist_km  = dist_graus * 111.0;
            double time_min = dist_km * 2.0;
            // Adiciona fator de tortuosidade urbana (15%)
            g.time_matrix[i][j] = time_min * 1.15;
        }
    }

    return g;
}

// ----------------------------------------------------------------
// Cria instancia pequena (10 nos) para o algoritmo exato
// ----------------------------------------------------------------
Graph create_small_instance() {
    const int N = 10;
    Graph g(N);
    // Subconjunto dos 10 nos mais importantes de Boa Vista
    struct NodeData { const char* name; double demand; double lat; double lon; };
    NodeData nodes[] = {
        {"Terminal Central",     0,   2.8235, -60.6758},
        {"UFRR Paricarana",    300,   2.8503, -60.7169},
        {"HGR",                280,   2.8197, -60.6892},
        {"Shopping Canarinho", 250,   2.8341, -60.6631},
        {"Mercado Municipal",  220,   2.8218, -60.6736},
        {"Aeroporto",          200,   2.8558, -60.6902},
        {"IFRR",               150,   2.8087, -60.6801},
        {"Bairro Caranã",      180,   2.7956, -60.6998},
        {"Centro Cívico",      145,   2.8234, -60.6723},
        {"Distrito Industrial",160,   2.9012, -60.6845}
    };
    for (int i = 0; i < N; i++) {
        g.nodes[i].name   = nodes[i].name;
        g.nodes[i].demand = nodes[i].demand;
        g.nodes[i].lat    = nodes[i].lat;
        g.nodes[i].lon    = nodes[i].lon;
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            if (i == j) { g.time_matrix[i][j] = 0.0; continue; }
            double dlat = nodes[i].lat - nodes[j].lat;
            double dlon = nodes[i].lon - nodes[j].lon;
            double dist = std::sqrt(dlat*dlat + dlon*dlon) * 111.0 * 2.0 * 1.15;
            g.time_matrix[i][j] = dist;
        }
    return g;
}

// ----------------------------------------------------------------
// Imprime uma rota com nomes dos nos
// ----------------------------------------------------------------
void print_route(const Graph& g, const std::vector<int>& route) {
    for (int i = 0; i < (int)route.size(); i++) {
        std::cout << g.nodes[route[i]].name;
        if (i + 1 < (int)route.size()) std::cout << " -> ";
    }
    std::cout << "\n";
}

// ----------------------------------------------------------------
// Experimento com a heuristica gulosa para varios valores de alpha/beta
// Gera uma fronteira de Pareto aproximada
// ----------------------------------------------------------------
ParetoFront run_greedy_pareto(const Graph& g) {
    std::cout << "\n=== Heuristica Gulosa (Variando alpha/beta) ===\n";
    GreedySolver gs;
    ParetoFront pf;

    // Variar os pesos para explorar o espaco de Pareto
    // alpha = peso do tempo, beta = peso da demanda
    // Total de combinacoes: 11 x 11 = 121 chamadas O(n^2) cada
    for (int a = 0; a <= 10; a++) {
        for (int b = 0; b <= 10; b++) {
            if (a == 0 && b == 0) continue;
            double alpha = a / 10.0;
            double beta  = b / 10.0;

            auto sol = gs.solve(g, alpha, beta, true);
            if (sol.total_time < INF && sol.total_demand > 0) {
                ParetoPoint pp;
                pp.route  = sol.route;
                pp.time   = sol.total_time;
                pp.demand = sol.total_demand;
                pp.alpha  = alpha;
                pp.beta   = beta;
                pf.add(pp);
            }
        }
    }
    return pf;
}

// ----------------------------------------------------------------
// Exporta historico de convergencia do NSGA-II para CSV
// ----------------------------------------------------------------
void export_hv_history(const std::vector<double>& hv, const std::string& fn) {
    std::ofstream f(fn);
    f << "geracao,hipervolume\n";
    for (int i = 0; i < (int)hv.size(); i++)
        f << i << "," << hv[i] << "\n";
    f.close();
    std::cout << "Historico HV exportado para: " << fn << "\n";
}

// ----------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------
int main() {
    std::cout << "================================================\n";
    std::cout << " MO-TSP: Rotas de Transporte Publico Boa Vista\n";
    std::cout << " DCC606 - Analise de Algoritmos - UFRR 2026\n";
    std::cout << "================================================\n\n";

    // ==============================================================
    // EXPERIMENTO 1: Algoritmo Exato (Branch-and-Bound) - instancia pequena
    // ==============================================================
    sep();
    std::cout << "EXPERIMENTO 1: Algoritmo Exato (Branch-and-Bound)\n";
    std::cout << "Instancia: 10 nos (subconjunto Boa Vista)\n";
    sep();

    Graph g_small = create_small_instance();

    BranchAndBound bb;
    double alpha_bb = 1.0, beta_bb = 0.5;

    auto t0 = Clock::now();
    auto exact_sol = bb.solve(g_small, alpha_bb, beta_bb);
    double ms_bb = MS(Clock::now() - t0).count();

    std::cout << "Rota otima: ";
    print_route(g_small, exact_sol.route);
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Tempo total: " << exact_sol.total_time << " min\n";
    std::cout << "Demanda total: " << exact_sol.total_demand << " passageiros\n";
    std::cout << "Nos explorados na arvore B&B: " << exact_sol.nodes_explored << "\n";
    std::cout << "Tempo de CPU: " << ms_bb << " ms\n";
    std::cout << "Complexidade: O(n!) pior caso, poda reduziu para "
              << exact_sol.nodes_explored << " nos\n";

    // ==============================================================
    // EXPERIMENTO 2: Heuristica Gulosa - instancia Boa Vista (30 nos)
    // ==============================================================
    sep();
    std::cout << "EXPERIMENTO 2: Heuristica Gulosa + 2-opt\n";
    std::cout << "Instancia: Boa Vista com 30 nos\n";
    sep();

    Graph g_bv = create_boa_vista();
    GreedySolver gs;

    auto t1 = Clock::now();
    ParetoFront pf_greedy = run_greedy_pareto(g_bv);
    double ms_greedy = MS(Clock::now() - t1).count();

    pf_greedy.print();
    std::cout << "\nHipervolume (Greedy): " << pf_greedy.hypervolume() << "\n";
    std::cout << "Tempo de CPU total: " << ms_greedy << " ms\n";
    std::cout << "Complexidade: O(n^2) por solucao, 121 combinacoes de pesos\n";
    pf_greedy.export_csv("results/pareto_greedy.csv");

    // Mostrar a melhor solucao balanceada (alpha=0.5, beta=0.5)
    auto sol_balanced = gs.solve(g_bv, 0.5, 0.5, true);
    std::cout << "\nSolucao balanceada (alpha=0.5, beta=0.5):\n";
    std::cout << "Tempo: " << sol_balanced.total_time << " min | ";
    std::cout << "Demanda: " << sol_balanced.total_demand << " pass\n";

    // ==============================================================
    // EXPERIMENTO 3: NSGA-II - instancia Boa Vista (30 nos)
    // ==============================================================
    sep();
    std::cout << "EXPERIMENTO 3: NSGA-II Multiobjetivo\n";
    std::cout << "Instancia: Boa Vista com 30 nos\n";
    sep();

    NSGA2Config nsga_cfg;
    nsga_cfg.pop_size    = 50;
    nsga_cfg.generations = 100;
    nsga_cfg.seed        = 42;

    NSGA2 nsga(nsga_cfg);

    auto t2 = Clock::now();
    ParetoFront pf_nsga = nsga.solve(g_bv);
    double ms_nsga = MS(Clock::now() - t2).count();

    pf_nsga.print();
    std::cout << "\nHipervolume (NSGA-II): " << pf_nsga.hypervolume() << "\n";
    std::cout << "Tempo de CPU: " << ms_nsga << " ms\n";
    std::cout << "Complexidade: O(generations * pop_size^2) = O("
              << nsga_cfg.generations << " * " << nsga_cfg.pop_size << "^2)\n";

    pf_nsga.export_csv("results/pareto_nsga2.csv");
    export_hv_history(nsga.hv_history, "results/hv_convergence.csv");

    // ==============================================================
    // TABELA RESUMO (formato do projeto)
    // ==============================================================
    sep();
    std::cout << "TABELA 6: Protocolo de Registro de Ensaios Experimentais\n";
    sep();
    printf("%-30s | %4s | %-20s | %10s | %10s\n",
           "Instancia / Cenario", "|V|", "Algoritmo", "HV", "CPU (ms)");
    sep();
    printf("%-30s | %4d | %-20s | %10.4f | %10.2f\n",
           "Boa Vista - 30 nos", 30, "Greedy + 2-opt",
           pf_greedy.hypervolume(), ms_greedy);
    printf("%-30s | %4d | %-20s | %10.4f | %10.2f\n",
           "Boa Vista - 30 nos", 30, "NSGA-II",
           pf_nsga.hypervolume(), ms_nsga);
    printf("%-30s | %4d | %-20s | %10s | %10.2f\n",
           "Boa Vista - 10 nos (exato)", 10, "Branch-and-Bound",
           "-", ms_bb);
    sep();

    std::cout << "\nArquivos de saida gerados em results/:\n";
    std::cout << "  pareto_greedy.csv   - Fronteira Pareto (Guloso)\n";
    std::cout << "  pareto_nsga2.csv    - Fronteira Pareto (NSGA-II)\n";
    std::cout << "  hv_convergence.csv  - Convergencia do NSGA-II\n";
    std::cout << "\nPara plotar: python3 ../plot_pareto.py\n";

    return 0;
}
