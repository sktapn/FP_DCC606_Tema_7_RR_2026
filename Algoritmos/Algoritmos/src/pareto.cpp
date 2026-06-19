#include "pareto.h"
#include <algorithm>
#include <cmath>

/*
 * pareto.cpp
 */

bool ParetoFront::dominates(const ParetoPoint& p1, const ParetoPoint& p2) const {
    // p1 domina p2 se:
    //   p1.time   <= p2.time   (nao e pior no tempo)
    //   p1.demand >= p2.demand (nao e pior na demanda)
    // com pelo menos uma diferenca estrita
    bool at_least_as_good_time   = (p1.time   <= p2.time);
    bool at_least_as_good_demand = (p1.demand >= p2.demand);
    bool strictly_better_time    = (p1.time   <  p2.time);
    bool strictly_better_demand  = (p1.demand >  p2.demand);

    return (at_least_as_good_time && at_least_as_good_demand) &&
           (strictly_better_time  || strictly_better_demand);
}

void ParetoFront::add(const ParetoPoint& p) {
    // Verificar se p e dominado por algum ponto existente
    for (const auto& existing : points) {
        if (dominates(existing, p)) return; // p e dominado, nao adicionar
    }

    // Remover pontos que p domina
    points.erase(
        std::remove_if(points.begin(), points.end(),
            [&](const ParetoPoint& existing) {
                return dominates(p, existing);
            }),
        points.end()
    );

    points.push_back(p);
}

void ParetoFront::print() const {
    std::cout << "\n=== Fronteira de Pareto (" << points.size() << " solucoes) ===\n";
    std::cout << "  #  | Tempo (min) | Demanda (pass) | Alpha | Beta\n";
    std::cout << "-----|-------------|----------------|-------|-----\n";
    for (int i = 0; i < (int)points.size(); i++) {
        printf("  %2d | %11.1f | %14.0f | %5.2f | %4.2f\n",
               i+1, points[i].time, points[i].demand,
               points[i].alpha, points[i].beta);
    }
}

double ParetoFront::hypervolume() const {
    /*
     * Calculo do hipervolume 2D (area dominada pela fronteira)
     * relativo ao ponto de referencia (pior caso):
     *   ref = (max_time * 1.1, 0)
     *
     * Algoritmo: ordena por tempo (crescente), soma fatias retangulares.
     * Complexidade: O(m log m)
     */
    if (points.empty()) return 0.0;

    // Ponto de referencia: pior tempo * 1.1, demanda = 0
    double max_time = 0;
    for (const auto& p : points) max_time = std::max(max_time, p.time);
    double ref_time = max_time * 1.1;

    // Ordenar por tempo crescente
    std::vector<ParetoPoint> sorted = points;
    std::sort(sorted.begin(), sorted.end(),
              [](const ParetoPoint& a, const ParetoPoint& b) {
                  return a.time < b.time;
              });

    // Somar areas das fatias: largura = diff de tempo, altura = demanda
    double hv = 0.0;
    double max_demand = 0;
    for (const auto& p : sorted) max_demand = std::max(max_demand, p.demand);

    // Normalizar: hv em [0,1]
    // Varrer da esquerda para direita
    for (int i = 0; i < (int)sorted.size(); i++) {
        double next_time = (i + 1 < (int)sorted.size()) 
                           ? sorted[i+1].time : ref_time;
        double width     = next_time - sorted[i].time;
        double height    = sorted[i].demand;
        hv += width * height;
    }

    // Normalizar pelo espaco de busca total
    double total_area = ref_time * max_demand;
    return (total_area > 0) ? hv / total_area : 0.0;
}

void ParetoFront::export_csv(const std::string& filename) const {
    std::ofstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Erro ao exportar CSV: " << filename << "\n";
        return;
    }
    f << "tempo,demanda,alpha,beta\n";
    for (const auto& p : points) {
        f << p.time << "," << p.demand << ","
          << p.alpha << "," << p.beta << "\n";
    }
    f.close();
    std::cout << "Fronteira de Pareto exportada para: " << filename << "\n";
}
