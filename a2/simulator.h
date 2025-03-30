#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "network.h"
#include <cassert>
#include "helper.h"

class Peer;

class Sim // simulator class
{
private:
    OverlayNetwork *malNet;
    Network *normNet;

public:
    double malFraction; // simulation parameters
    int numPeers, simTime;
    vector<Peer *> peers; // all peers in the simulation

    Sim(int numPeers, double malFraction, float simTime)
        : numPeers(numPeers), malFraction(malFraction), simTime(simTime)
    {
        vector<int> indices;
        normNet = new Network(numPeers);
        for (int i = 0; i < numPeers; i++)
        {
            indices.push_back(i);
        }
        vector<vector<int>> graph = generate_graph(numPeers, indices);
        for (int i = 0; i < numPeers; i++)
        {
            peers.push_back(new HonestPeer(i, normNet));
            peers[i]->neighbours = graph[i];
            peers[i]->hash_power = 1.0 / double(numPeers);
            peers[i]->slow = true;
        }
        
        
        vector<int> mal_idx = randomIndices(int(malFraction * numPeers), numPeers);

        malNet = new OverlayNetwork(numPeers, mal_idx);

        int cnt = 0;
        for (auto i : mal_idx)
        {
            free(peers[i]);
            peers[i] = new MaliciousPeer(i, cnt, normNet, malNet);
            peers[i]->slow = false;
            cnt++;
        }
        malNet->peers = peers;
        normNet->peers = peers;
        malNet->assignLinkSpeed();
        normNet->assignLinkSpeed();
        malNet->assignPropDelay();
        normNet->assignPropDelay();
        graph = generate_graph(numPeers, mal_idx);
        for (auto i : mal_idx)
        {
            malNet->malicious_peers.insert(i);
        }

        for (auto i : mal_idx)
        {
            peers[i]->malicious_neighbours = graph[i];
        }
    }
    void start();
};

#endif
