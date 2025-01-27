#include <bits/stdc++.h>
using namespace std;

vector<vector<int>> graph;
int num_peers = 10;

int generate_random_number(int lower_bound, int upper_bound)
{
    return lower_bound + rand() % (upper_bound - lower_bound + 1);
}

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

bool is_graph_valid()
{
    return is_graph_connected() && are_degrees_correct();
}

void generate_graph()
{
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
}

int main()
{
    srand(time(0));

    num_peers = 10;

    generate_graph();

    for (int i = 0; i < graph.size(); i++)
    {
        cout << "Peer " << i << ": ";
        for (int neighbor : graph[i])
        {
            cout << neighbor << " ";
        }
        cout << endl;
    }

    return 0;
}