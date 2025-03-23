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
#include "helper.h"
using namespace std;

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

class Transaction
{
public:
    int txID;
    int sender_id;
    int receiver_id;
    int amount;

    Transaction(int sender_id, int receiver_id, int amt)
    {
        this->txID = txnIDctr++;
        this->sender_id = sender_id;
        this->receiver_id = receiver_id;
        this->amount = amt;
    }
};

class Block
{
public:
    int BlkID;
    int miner_id;
    int parent_id;
    vector<string> txns;

    Block(int miner_id, int parent_id, vector<string> txns)
    {
        this->BlkID = blkIDctr++;
        this->miner_id = miner_id;
        this->parent_id = parent_id;
        this->txns = txns;
    }
};

// extern unordered_map<int, Block *> globalBlocks;             // maps block id to block ptr
extern unordered_map<int, Transaction *> globalTransactions; // maps transaction id to transaction ptr

class treeNode // for the nodes of the tree stored by each peer
{
public:
    int depth;
    treeNode *parent_ptr;
    int block_id;
    map<int, int> balances; // peerID to balances at this node

    treeNode(treeNode *parent_node, Block *blk)
    {
        this->depth = parent_node ? parent_node->depth + 1 : 0;
        this->parent_ptr = parent_node;
        this->block_id = blk->BlkID;
    }
};

class P2P // simulator class
{
public:
    int I, Ttx, Tt; // simulation parameters
    int numPeers;
    vector<Peer> peers;                // all peers in the simulation
    vector<vector<double>> link_speed; // link speeds of all peers
    vector<vector<double>> prop_delay; // prop delay of all peers
    int max_txn, max_block;
    int total_transactions;
    int forks;
    Sim *sim;
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> sendingQueue;     // {timestamp, t(0)/b(1)/h(2)/g(3), rcv_id}, msg
    priority_queue<vector<int>, vector<vector<int>>, Compare> transactionQueue; // {timestamp, sender}
    priority_queue<vector<int>, vector<vector<int>>, Compare> blockQueue;       // {timestamp, block_ID, sender}
    priority_queue<vector<int>, vector<vector<int>>, Compare> timeoutQueue;      // {timestamp, waiting peer ID, hash}
    priority_queue<pair<vector<int>, string>, vector<pair<vector<int>, string>>, Compare2> timeoutQueue;     // {timestamp, waiting peer ID, hash}

    void assignSlowFast();
    void assignCPU();
    void assignPropDelay();
    void assignLinkSpeed();
    void computeHashPower();
    void start();
    void run(long long);
    int calculateLatency(int, int, double);
    
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
        // simTime = st;
        sendingQueue = {};
        transactionQueue = {};
        blockQueue = {};
        vector<vector<int>> graph = generate_graph(np);


        for (int i = 0; i < numPeers; i++)
        {
            peers[i].neighbours = graph[i];
        }

        assignSlowFast();
        assignCPU();
        computeHashPower();
        assignPropDelay();
        assignLinkSpeed();
    }
};

class OverlayNetwork : public P2P
{
public:
    set<int> malicious_peers;
    int mined_length;

    OverlayNetwork(int np, set<int> mal_idx)
    {
        total_transactions = 0;
        forks = 0;
        max_txn = 0;
        max_block = 0;
        sendingQueue = {};
        transactionQueue = {};
        blockQueue = {};
        
        vector<vector<int>> graph = generate_graph(np);

        malicious_peers = mal_idx; // contains peerIDs of all mal peers

        for (int i = 0; i < numPeers; i++)
        {
            peers[i].neighbours = graph[i];
        }
        assignSlowFast();
        assignCPU();
        computeHashPower();
        assignPropDelay();
        assignLinkSpeed();
    }
};

#endif