#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>

const double INF = 1e9;

struct Node {
    int id;
    std::string name;
    double demand, lat, lon;
};

class Graph {
public:
    int n;
    std::vector<Node> nodes;
    std::vector<std::vector<double>> time_matrix;

    Graph(int n);
    int size() const { return n; }
    void set_demand(int id, double demand);
    void set_time(int from, int to, double time);
    bool load_from_file(const std::string& filename);
    void print_time_matrix() const;
};
#endif