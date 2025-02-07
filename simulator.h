#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <queue>
#include <unordered_map>
#include <map>
#include <set>
#include <iostream>

#include "helper.h"
using namespace std;

struct Compare {
    bool operator()(const vector<int>& a, const vector<int>& b) {
        return a > b; // Min-heap: smallest element first (lexicographical order)
    }
};

extern int txnIDctr;
extern int blkIDctr;
extern int curr_time;
extern priority_queue<vector<int>, vector<vector<int>>, Compare> sendingQueue;     // {timestamp, t(0)/b(1), ID, rcv}
extern priority_queue<vector<int>, vector<vector<int>>, Compare> transactionQueue;
extern priority_queue<vector<int>, vector<vector<int>>, Compare> blockQueue;

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
    int BlkID; // check if receiving peer has already seen the block
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

extern unordered_map<int, Block *> globalBlocks;             // maps block id to block
extern unordered_map<int, Transaction *> globalTransactions; // maps transaction id to transaction - populate later on ie when txn is popped from queue

class treeNode // for the nodes of the tree stored by each peer
{
public:
    int depth;
    treeNode *parent_ptr;
    int block_id;
    map<int, int> balances; // peerID to curr balance

    treeNode(treeNode *parent_node, Block *blk)
    {
        this->depth = parent_node ? parent_node->depth + 1 : 0;
        this->parent_ptr = parent_node;
        this->block_id = blk->BlkID;
        if (parent_node)
        {
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
    }
};

class P2P;

class Peer
{
public:
    int longestChain; // leaf block ID
    // int blockchainLeaf;
    int peerID;
    bool slow;
    bool lowCPU;
    bool coinbase;
    int maxDepth;
    double hash_power;
    int total_blocks;
    int total_transactions;
    set<int> memPool;               // mempool stroes txn IDs
    set<int> blockSet;              // stores ids of blocks seen by peer
    set<int> leafBlocks;            // stores ids of leaf blocks
    set<int> transactionSet;        // stores ids of transactions seen by peer
    treeNode *genesis_blk;          // root of the tree
    map<int, treeNode *> blockTree; // maps blockID to the corresponding treeNode
    set<int> orphanBlocks;          // set of orphan blocks
    P2P *simulator;

    // tree at each peer?
    vector<int> neighbours;
    // queue<int> transactionQueue;

    Peer(int pID, P2P *simulator)
    {
        total_blocks = 0;
        total_transactions = 0;
        peerID = pID;
        // coinbase = false;
        longestChain = 0;
        this->simulator = simulator;

    }

    void setHashPower(double);
    void setlowCPU();
    void setslow();
    void generateTransaction();
    void broadcastTransaction();
    void receiveTransaction(int);
    void generateBlock();
    void receiveBlock(int);
    bool verifyBlock(int);
    void broadcastBlock(int);
    void processOrphanBlocks(Block &);
    bool validateBlock(Block &, map<int, int> &);
};

class P2P
{
public:
    int z0, z1;
    vector<Peer> peers;
    vector<vector<double>> link_speed;
    vector<vector<double>> prop_delay;
    int numPeers;
    int simTime;
    int max_txn, max_block;
    int Ttx;
    int I;
    int total_blocks;
    int total_transactions;
    int forks;
    P2P(int np)
    {
        total_blocks = 0;
        total_transactions = 0;
        forks = 0;
        cout << "P2P constructor called" << endl;
        numPeers = np;
        max_txn = 0;
        max_block = 0;
        vector<vector<int>> graph = generate_graph(numPeers);
        cout << "Graph generated" << endl;
        for (int i = 0; i < numPeers; i++)
        {
            peers.push_back(Peer(i, this));
            peers[i].neighbours = graph[i];
        }
        assignSlowFast();
        assignCPU();
        computeHashPower();
        assignPropDelay();
        assignLinkSpeed();
        cout << "Going out" << endl;
    } // default constructor

    P2P(int z0, int z1, int np, int I, int Ttx) : z0(z0), z1(z1), numPeers(np)
    {
        max_txn = 0;
        max_block = 0;
        vector<vector<int>> graph = generate_graph(numPeers);
        for (int i = 0; i < numPeers; i++)
        {
            peers.push_back(Peer(i, this));
            peers[i].neighbours = graph[i];
        }
        this->I = I;
        this->Ttx = Ttx;
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

#endif
