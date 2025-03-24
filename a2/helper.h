#ifndef HELPER_H
#define HELPER_H
#include <iostream>
#include <queue>
#include <set>
#include <vector>
#include <random>
using namespace std;

// header file for all helper functions

int generateExponential(double Ttx);

double sampleUniform(double a, double b);

vector<int> randomIndices(int x, int n);

int chooseRandomPeer(vector<int> &peers);

int generate_random_number(int lower_bound, int upper_bound);

vector<vector<int>> generate_graph(int num_peers_l, vector<int> &peers);

string calculateHash(int blkid, int miner_id, string parent_hash, vector<string> txns);

vector<int> parse_txn(const string& transaction);

string construct_txn(const vector<int> &transaction);

string construct_coinbase(int peerID, int txn_id);

#endif
