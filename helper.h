#ifndef HELPER_H
#define HELPER_H
#include <iostream>
#include <queue>
#include <random>
using namespace std;

int generateExponential(double Ttx);

double sampleUniform(double a, double b);

vector<int> randomIndices(int x, int n);

int generate_random_number(int lower_bound, int upper_bound);

vector<vector<int>> generate_graph(int num_peers_l);

#endif
