#ifndef PARETO_H
#define PARETO_H
#include <vector>
#include <iostream>
#include <fstream>

struct ParetoPoint {
    std::vector<int> route;
    double time, demand, alpha, beta;
};

class ParetoFront {
public:
    std::vector<ParetoPoint> points;
    bool dominates(const ParetoPoint& p1, const ParetoPoint& p2) const;
    void add(const ParetoPoint& p);
    void print() const;
    double hypervolume() const;
    void export_csv(const std::string& filename) const;
    int size() const { return points.size(); }
};
#endif