#include "helper.h"
#include <algorithm>
#include <set>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
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
set<int> randomIndices(int x, int n)
{
    if (x > n)
    {
        return {};
    }

    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, n - 1);

    set<int> indices;
    while (indices.size() < x)
    {
        indices.insert(dist(gen));
    }

    return indices;
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
bool is_graph_connected()
{
    vector<bool> vis(num_peers, 0);
    queue<int> q;
    q.push(0);
    vis[0] = true;
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

    return (peers_visited == num_peers);
}

// checks is every node has a degree between 3 and 6 (as per the problem statement)
bool are_degrees_correct()
{
    for (int i = 0; i < num_peers; ++i)
    {
        if (graph[i].size() < 3 || graph[i].size() > 6)
        {
            return false;
        }
    }
    return true;
}

// checks if the graph is connected as well as satisfies the given degree constraints
bool is_graph_valid()
{
    return is_graph_connected() && are_degrees_correct();
}

// main function to generate random graphs
// keeps generating until it finds a valid graph and finally return the adjacency list
vector<vector<int>> generate_graph(int num_peers_l)
{
    num_peers = num_peers_l;
    do
    {
        vector<int> degrees;
        for (int i = 0; i < num_peers; ++i)
        {
            degrees.push_back(generate_random_number(3, 6));
        }
        graph = {};
        for (int i = 0; i < num_peers; i++)
        {
            graph.push_back(vector<int>());
        }
        for (int i = 0; i < num_peers; i++)
        {
            for (int j = i + 1; j < num_peers; j++)
            {
                if (degrees[i] > 0 && degrees[j] > 0)
                {
                    degrees[i]--;
                    degrees[j]--;
                    graph[i].push_back(j);
                    graph[j].push_back(i);
                }
            }
        }
    } while (!is_graph_valid());

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