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
extern bool eclipse_attack;

class P2P // network class
{
public:
    bool stop;
    virtual ~P2P() = default;
    int I, Ttx, Tt, numPeers;          // simulation parameters
    vector<Peer *> peers;              // all peers in the simulation
    vector<vector<double>> link_speed; // link speeds of all peers
    vector<vector<double>> prop_delay; // prop delay of all peers
    int max_txn, max_block;            // counter of transactions and blocks in network
    int total_transactions;            // total transactions in the network
    int forks;
    Sim *sim;                          // pointer to the main simulator
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> sendingQueue; // {{timestamp, t(0)/b(1)/h(2)/g(3)/broadcastprivatechain(4), sender_id, rcv_id}, msg}
    priority_queue<vector<int>, vector<vector<int>>, Compare> transactionQueue;                          // {timestamp, sender}
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> blockQueue;   // {{timetamp, sender}, hash}
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> timeoutQueue; // {timestamp, waiting peer ID, hash}

    int calculateLatency(int, int, double);

    virtual void assignPropDelay() = 0;
    void assignLinkSpeed();
    virtual void run(long long) = 0;
    virtual void clearRun() = 0;
};


/*
class for original network
*/
class Network : public P2P
{
public:
    int mined_length;
    Network(int np);

    ~Network() {}
    void run(long long) override;
    void clearRun() override;
    void assignPropDelay() override;

};

/*
class for overlay (maliciousx) network
*/
class OverlayNetwork : public P2P
{
public:
    string attack_root; // hash val of attack root
    set<int> malicious_peers;
    int ringmasterID;
    int malicious_len;
    string malicious_leaf;

    OverlayNetwork(int np, vector<int> mal_idx);

    ~OverlayNetwork() {}

    void run(long long) override;
    void clearRun() override;
    void assignPropDelay() override;

};

#endif