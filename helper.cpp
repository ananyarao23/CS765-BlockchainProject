#include "helper.h"
#include <algorithm>

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

    for (int idx : indices)
    {
        cout << idx << " ";
    }
    cout << endl;

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
int num_peers; // number of peers in network


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