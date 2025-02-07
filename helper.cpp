# include "helper.h"
#include <algorithm>

using namespace std;
int generateExponential(double Ttx)
{
    // cout<<"Ttx: "<<Ttx<<endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> dist(1.0 / Ttx); // lambda = 1 / mean
    // cout<<"exponential generated"<<endl;
    return (int)dist(gen);
}

double sampleUniform(double a, double b)
{
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister RNG

    // Define the uniform distribution in range [a, b]
    std::uniform_real_distribution<double> dist(a, b);

    // cout<<"uniform generated"<<endl;
    // Generate and return a random sample
    return dist(gen);
}

vector<int> randomIndices(int x, int n)
{
    // cout << "Generating indices... "<<x<< endl;
    
    if (x > n) {
        cerr << "Error: x cannot be greater than n!" << endl;
        return {};
    }

    static random_device rd;
    static mt19937 gen(rd());

    vector<int> allIndices(n);
    for (int i = 0; i < n; ++i) {
        allIndices[i] = i;
    }

    shuffle(allIndices.begin(), allIndices.end(), gen);

    vector<int> indices(allIndices.begin(), allIndices.begin() + x);

    for (int idx : indices) {
        cout << idx << " ";
    }
    cout << endl;

    return indices;
}

int generate_random_number(int lower_bound, int upper_bound)
{
    // cout<<"random generated"<<endl;
    return lower_bound + rand() % (upper_bound - lower_bound + 1);
}

vector<vector<int>> graph;
int num_peers = 10;


bool is_graph_connected()
{
    // cout<<"graph connected called"<<endl;
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

    // cout<<"graph connected checked"<<endl;
    return (peers_visited == num_peers);
}

bool are_degrees_correct()
{
    // cout<<"degs crct called"<<endl;
    for (int i = 0; i < num_peers; ++i)
    {
        if (graph[i].size() < 3 || graph[i].size() > 6)
        {
            // cout<<"degs done"<<endl;
            return false;
        }
    }
    // cout<<"true degs done"<<endl;
    return true;
}

bool is_graph_valid()
{
    return is_graph_connected() && are_degrees_correct();
}

vector<vector<int>> generate_graph(int num_peers_l)
{
    // cout<<"graph generated"<<endl;
    num_peers = num_peers_l;
    // cout<<num_peers<<endl;
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
    // cout<<"graph actually generated"<<endl;
    return graph;
}