#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "network.h"
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
        normNet = new Network(numPeers);
        for (int i = 0; i < numPeers; i++)
        {
            peers.push_back(new HonestPeer(i, normNet));
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
    }
    void start();
};

#endif
