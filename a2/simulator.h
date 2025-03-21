#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "network.h"
#include "helper.h"

class Peer;

class Sim // simulator class
{
private:
    P2P *MalNet;
    P2P *NormNet;

public:
    float malPercent, I, Ttx; // simulation parameters
    int numPeers, simTime;
    vector<Peer *> peers; // all peers in the simulation

    Sim(int numPeers, float malPercent, float I, float Ttx, float simTime)
        : numPeers(numPeers), malPercent(malPercent), I(I), Ttx(Ttx), simTime(simTime)
    {
        NormNet = new P2P(numPeers, {});
        for (int i = 0; i < numPeers; i++)
        {
            peers[i] = new Peer(i,-1, NormNet, nullptr);
            peers[i]->hash_power = 1.0 / float(numPeers);
            peers[i]->slow = true;
        }
        set<int> mal_idx = randomIndices(numPeers, malPercent);

        MalNet = new P2P(int(numPeers * malPercent),mal_idx);
        int cnt = 0;
        for (auto i : mal_idx)
        {
            free(peers[i]);
            peers[i] = new Peer(i,cnt, NormNet, MalNet);
            peers[i]->slow = false;
            cnt++;
        }
    }
};

#endif
