#ifndef NETWORK_H
#define NETWORK_H

#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <queue>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include "structure.h"
using namespace std;

class Sim;

struct Compare // for min priority queue implementation
{
    bool operator()(const vector<int> &a, const vector<int> &b)
    {
        return a > b;
    }
};

struct Compare2 // for min priority queue implementation for pair<vector<int>, string>
{
    bool operator()(const pair<vector<int>, string> &a, const pair<vector<int>, string> &b)
    {
        return a.first > b.first;
    }
};

extern int txnIDctr; // for setting txn ID
extern int blkIDctr; // for setting block ID

class P2P // simulator class
{
public:
    virtual ~P2P() = default;
    int I, Ttx, Tt, numPeers;          // simulation parameters
    vector<Peer *> peers;              // all peers in the simulation
    vector<vector<double>> link_speed; // link speeds of all peers
    vector<vector<double>> prop_delay; // prop delay of all peers
    int max_txn, max_block;
    int total_transactions;
    int forks;
    Sim *sim;
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> sendingQueue; // {timestamp, t(0)/b(1)/h(2)/g(3), sender_id, rcv_id}, msg
    priority_queue<vector<int>, vector<vector<int>>, Compare> transactionQueue;                          // {timestamp, sender}
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> blockQueue;
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> timeoutQueue; // {timestamp, waiting peer ID, hash}

    int calculateLatency(int, int, double);

    // virtual void assignSlowFast() = 0;
    // virtual void assignCPU() = 0;
    void assignPropDelay();
    void assignLinkSpeed();

    // virtual void computeHashPower();
    virtual void run(long long) = 0;
};

class Network : public P2P
{
public:
    int mined_length;
    Network(int np)
    {
        total_transactions = 0;
        forks = 0;
        max_txn = 0;
        max_block = 0;
        sendingQueue = {};
        transactionQueue = {};
        blockQueue = {};
        numPeers = np;
        // assignPropDelay();
        // assignLinkSpeed();
    }

    ~Network() {}
    void run(long long) override;
    // virtual void assignSlowFast() = 0;
    // virtual void assignCPU() = 0;
    // virtual void assignPropDelay() = 0;
    // virtual void assignLinkSpeed() = 0;
};

class OverlayNetwork : public P2P
{
public:
    set<int> malicious_peers;
    int ringmasterID;

    OverlayNetwork(int np, vector<int> mal_idx)
    {
        total_transactions = 0;
        forks = 0;
        max_txn = 0;
        max_block = 0;
        sendingQueue = {};
        transactionQueue = {};
        blockQueue = {};
        numPeers = np;
        ringmasterID = chooseRandomPeer(mal_idx);
        cout << "Malicious peers: ";
        for (auto i : mal_idx)
        {
            cout << i << " ";
            malicious_peers.insert(i);
        }
        cout << endl;
        cout << "Ringmaster ID: " << ringmasterID << endl;
        // assignLinkSpeed();
        // assignPropDelay();
    }

    ~OverlayNetwork() {}
    void run(long long) override;
};

#endif