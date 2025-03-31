#include "helper.h"
#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <iomanip>
#include <random>

using namespace std;

/*
Generates a sample from an exponential distribution with mean Ttx
*/
int generateExponential(double Ttx)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> dist(1.0 / Ttx); // lambda = 1 / mean

    return (int)dist(gen);
}

/*
Generates sa sample from a uniform distribution with lower bound = a, upper bound = b
*/
double sampleUniform(double a, double b)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(a, b);

    return dist(gen);
}

/*
Returns x random indices from [0..n-1]
*/
vector<int> randomIndices(int x, int n)
{
    if (x > n)
    {
        cerr << "Error: x cannot be greater than n!" << endl;
        return {};
    }

    static random_device rd;
    static mt19937 gen(rd());

    vector<int> allIndices(n);
    for (int i = 0; i < n; ++i)
    {
        allIndices[i] = i;
    }

    shuffle(allIndices.begin(), allIndices.end(), gen);

    vector<int> indices(allIndices.begin(), allIndices.begin() + x);

    return indices;
}

int chooseRandomPeer(vector<int> &peers)
{
    return peers[rand() % peers.size()];
}

/*
Generates a random number between lower_bound and upper_bound
*/
int generate_random_number(int lower_bound, int upper_bound)
{
    return lower_bound + rand() % (upper_bound - lower_bound + 1);
}

vector<vector<int>> graph; // global variable to store the adjaceny list of every node
int num_peers;             // number of peers in network

// returns true if the given graph is connected (all nodes are reachable by each other)
bool is_graph_connected(vector<int> &peers)
{
    vector<bool> vis(num_peers, 0);
    queue<int> q;
    q.push(peers[0]);
    vis[peers[0]] = true;
    int peers_visited = 1;

    while (!q.empty())
    {
        int curr = q.front();
        q.pop();

        for (auto neighbor : graph[curr])
        {
            if (!vis[neighbor])
            {
                vis[neighbor] = 1;
                q.push(neighbor);
                peers_visited++;
            }
        }
    }

    return (peers_visited == peers.size());
}

// checks is every node has a degree between 3 and 6 (as per the problem statement)
bool are_degrees_correct(vector<int> &peers)
{
    for (auto i : peers)
    {
        if (graph[i].size() < 3 || graph[i].size() > 6)
        {
            return false;
        }
    }
    return true;
}

// checks if the graph is connected as well as satisfies the given degree constraints
bool is_graph_valid(vector<int> &peers)
{
    return is_graph_connected(peers) && are_degrees_correct(peers);
}

// main function to generate random graphs
// keeps generating until it finds a valid graph and finally return the adjacency list
vector<vector<int>> generate_graph(int total_peers, vector<int> &peers)
{
    if (peers.size() < 4)
    {
        cout << "Need atleast 4 peers to generate graph!!" << endl;
        exit(1);
    }
    num_peers = total_peers;
    do
    {
        vector<int> degrees(total_peers, 0);
        for (auto u : peers)
        {
            degrees[u] = generate_random_number(3, 6);
        }
        graph = {};
        for (int i = 0; i < total_peers; i++)
        {
            graph.push_back(vector<int>());
        }
        for (int it = 0; it < peers.size(); it++)
        {
            int i = peers[it];
            for (int jt = it + 1; jt < peers.size(); jt++)
            {
                int j = peers[jt];
                if (degrees[i] > 0 && degrees[j] > 0)
                {
                    degrees[i]--;
                    degrees[j]--;
                    graph[i].push_back(j);
                    graph[j].push_back(i);
                }
            }
        }
    } while (!is_graph_valid(peers));

    return graph;
}

string calculateHash(int blkid, int miner_id, string parent_hash, vector<string> txns)
{
    stringstream ss;
    ss << blkid << miner_id << parent_hash;

    // Include transaction IDs in the hash computation
    for (string txnID : txns)
    {
        ss << txnID;
    }

    string input = ss.str();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)input.c_str(), input.size(), hash);

    stringstream hashStr;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        hashStr << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return hashStr.str();
}

vector<int> parse_txn(const string &transaction)
{
    vector<int> result;
    string txnIDStr, action, coins;
    int txnID, IDx, IDy, C;

    stringstream ss(transaction);
    ss >> txnIDStr;

    txnID = stoi(txnIDStr.substr(0, txnIDStr.size() - 1)); // Remove ':' and convert to int

    ss >> IDx >> action;

    if (action == "pays")
    {
        ss >> IDy >> C >> coins;
    }
    else if (action == "mines")
    {
        IDy = IDx;        // Miner is the recipient
        IDx = -1;         // Payer ID is set to -1
        ss >> C >> coins; // Read amount and "coins"
    }
    else
    {
        throw invalid_argument("Invalid transaction format");
    }

    result.push_back(txnID);
    result.push_back(IDx);
    result.push_back(IDy);
    result.push_back(C);

    return result;
}

string construct_txn(const vector<int> &transaction)
{
    if (transaction.size() != 4)
    {
        return "Invalid transaction data";
    }

    stringstream ss;
    ss << transaction[0] << ": " << transaction[1] << " pays " << transaction[2] << " " << transaction[3] << " coins";

    return ss.str();
}

string construct_coinbase(int peerID, int txn_id)
{
    stringstream ss;
    ss << txn_id << ": " << peerID << " mines 50 coins";
    return ss.str();
}

// {{block_id, miner_id}, {parent_hash, txns }}
pair<string, string> extract_root(string s)
{

    size_t pos1 = s.find('%');
    size_t pos2 = s.find('%', pos1 + 1);

    string s2 = s.substr(pos1 + 1, pos2 - pos1 - 1);
    string s3 = s.substr(pos2 + 1);
    return {s2, s3}; // Assuming you're returning a pair
}
