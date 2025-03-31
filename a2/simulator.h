#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "network.h"
#include <cassert>
#include "helper.h"

class Peer;


/*
Main simulator class 
    - runs both the networks 
    - stores attributes common to both networks
*/
class Sim 
{
private:
    Network *normNet;

public:
    OverlayNetwork *malNet;
    double malFraction; // simulation parameters
    int numPeers, simTime;
    vector<Peer *> peers; // all peers in the simulation

    Sim(int numPeers, double malFraction, float simTime)
        : numPeers(numPeers), malFraction(malFraction), simTime(simTime)
    {
        vector<int> indices;
        normNet = new Network(numPeers); // instantiate original network
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

        if (malFraction == 0)
        {
            normNet->peers = peers;
            normNet->assignLinkSpeed();
            normNet->assignPropDelay();
            return;
        }

        // choose random indices to make them malicious
        vector<int> mal_idx = randomIndices(int(malFraction * numPeers), numPeers);

        malNet = new OverlayNetwork(numPeers, mal_idx); // instantiate malicious network

        peers[malNet->ringmasterID]->hash_power *= int(malFraction * numPeers);
        int cnt = 0;
        for (auto i : mal_idx)
        {
            free(peers[i]);
            // populate the peer* vector with malicious peers
            peers[i] = new MaliciousPeer(i, cnt, normNet, malNet);
            peers[i]->slow = false;
            peers[i]->neighbours = graph[i];
            if (i != malNet->ringmasterID)
            {
                peers[i]->hash_power = 0.0;
            }
            else 
            {
                // ringmaster gets the hash power of all malicious peers
                peers[i]->hash_power = 1.0 / double(numPeers);
                peers[i]->hash_power *= int(malFraction * numPeers);
            }
            cnt++;
        }

        // populate the peers vectors of both networks
        malNet->peers = peers;
        normNet->peers = peers;

        // assign link speed and propagataion delay to nodes of both networks
        malNet->assignLinkSpeed();
        normNet->assignLinkSpeed();
        malNet->assignPropDelay();
        normNet->assignPropDelay();

        // generate malicious graph - following the degree constraint - 3 to 6
        graph = generate_graph(numPeers, mal_idx);

        for (auto i : mal_idx)
        {
            peers[i]->malicious_neighbours = graph[i];
        }
    }
    void start();
    void stop();
};

#endif
