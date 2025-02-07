#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <ctime>
#include <bits/stdc++.h>
#include <random>

#include "random_graph.h"
#include "helper.h"
using namespace std;

int txnIDctr = 0;
int blkIDctr = 0;

int curr_time;

priority_queue<vector<int>> sendingQueue;             // {timestamp, t(0)/b(1), ID, rcv}
priority_queue<vector<int>> transactionQueue;      // {timestamp, snd}
priority_queue<vector<int>> blockQueue;               // {timestamp, blkID, snd}
unordered_map<int, Block *> globalBlocks;             // maps block id to block
unordered_map<int, Transaction *> globalTransactions; // maps transaction id to transaction - populate later on ie when txn is popped from queue

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
    vector<int> txns;

    Block();

    Block(int miner_id, int parent_id, vector<int> txns)
    {
        this->BlkID = blkIDctr++;
        this->miner_id = miner_id;
        this->parent_id = parent_id;
        this->txns = txns;
    }
};

class treeNode // for the nodes of the tree stored by each peer
{
public:
    int depth;
    treeNode *parent_ptr;
    int block_id;
    map<int, int> balances; // peerID to curr balance

    treeNode(treeNode *parent_node, Block *blk)
    {
        this->depth = parent_node->depth + 1;
        this->parent_ptr = parent_node;
        this->block_id = blk->BlkID;

        Block *blk = globalBlocks[block_id];
        map<int, int> new_balances = parent_node->balances;
        new_balances[blk->miner_id] += 50;

        for (int txn_id : blk->txns)
        {
            Transaction *txn = globalTransactions[txn_id];
            new_balances[txn->sender_id] -= txn->amount;
            new_balances[txn->receiver_id] += txn->amount;
        }

        this->balances = new_balances;
    }
};


class Peer : public P2P
{
public:
    int longestChain; // leaf block ID
    int blockchainLeaf;
    int peerID;
    bool slow;
    bool lowCPU;
    bool coinbase;
    int maxDepth;
    set<int> memPool;               // mempool stroes txn IDs
    set<int> blockSet;              // stores ids of blocks seen by peer
    set<int> leafBlocks;            // stores ids of leaf blocks
    set<int> transactionSet;        // stores ids of transactions seen by peer
    treeNode *genesis_blk;          // root of the tree
    map<int, treeNode *> blockTree; // maps blockID to the corresponding treeNode
    set<int> orphanBlocks;          // set of orphan blocks
    

    // tree at each peer?

    double hash_power;
    bool lowCPU;
    vector<int> neighbours;
    // queue<int> transactionQueue;
    queue<int> blockQueue;

    Peer(int pID)
    {
        peerID = pID;
        coinbase = false;
        longestChain = -1;
    }
    void setHashPower(double);
    void setlowCPU();
    void setslow();
    void generateTransaction();
    void broadcastTransaction();
    void receiveTransaction(int);
    void generateBlock();
    void receiveBlock();
    bool verifyBlock(int);
    void broadcastBlock(int);
};

void Peer::setHashPower(double x)
{
    hash_power = x;
}

void Peer::setlowCPU()
{
    lowCPU = true;
}

void Peer::setslow()
{
    slow = true;
}

class P2P
{


public:
int z0, z1;
    vector<Peer> peers;
    vector<vector<double>> link_speed;
    vector<vector<double>> prop_delay;
    int numPeers;
    int max_txn, max_block;
    int Ttx;
    int I;
    P2P(); // default constructor
    P2P(int z0, int z1, int np,int I,int Ttx) : z0(z0), z1(z1), numPeers(np)
    {
        max_txn = 0;
        max_block = 0;
        vector<vector<int>> graph = generate_graph(numPeers);
        for (int i = 0; i < numPeers; i++)
        {
            peers.push_back(Peer(i));
            peers[i].neighbours = graph[i];
        }
        assignSlowFast();
        assignCPU();
        computeHashPower();
        assignPropDelay();
        assignLinkSpeed();
    }
    void assignSlowFast();
    void assignCPU();
    void assignPropDelay();
    void assignLinkSpeed();
    void computeHashPower();
    void start();
    int calculateLatency(int, int, double); // (i,j,size of message)
};



void P2P::assignSlowFast()
{
    int num = z0 * numPeers / 100;
    vector<int> indices = randomIndices(num, numPeers);
    for (int i : indices)
    {
        peers[i].setslow();
    }
}

void P2P::assignCPU()
{
    int num = z1 * numPeers / 100;
    vector<int> indices = randomIndices(num, numPeers);
    for (int i : indices)
    {
        peers[i].setlowCPU();
    }
}

void P2P::computeHashPower()
{
    double x;
    double coefficient = 0;
    for (int i = 0; i < numPeers; i++)
    {
        if (peers[i].lowCPU)
            coefficient += 1;
        else
            coefficient += 10;
    }
    x = 1.0 / coefficient;
    for (int i = 0; i < numPeers; i++)
    {
        if (peers[i].lowCPU)
            peers[i].setHashPower(x);
        else
            peers[i].setHashPower(x * 10);
    }
}



void P2P::assignPropDelay()
{
    for (int i = 0; i < num_peers; i++)
    {
        for (int j = 0; j < num_peers; j++)
        {
            prop_delay[i][j] = sampleUniform(0.01, 0.5);
        }
    }
}

void P2P::assignLinkSpeed()
{
    for (int i = 0; i < num_peers; i++)
    {
        for (int j = 0; j < num_peers; j++)
        {
            link_speed[i][j] = peers[i].slow || peers[j].slow ? 5 : 100;
        }
    }
}


// ms in megabits
int P2P::calculateLatency(int i, int j, double ms)
{
    double p = prop_delay[i][j];
    double c = link_speed[i][j];
    int d = generateExponential(0.096 / c);
    return p + d + ms / c;
}