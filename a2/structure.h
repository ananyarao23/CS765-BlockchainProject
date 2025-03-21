`   #ifndef SIMULATOR_H
#define SIMULATOR_H

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

extern int txnIDctr; // for setting txn ID
extern int blkIDctr; // for setting block ID
extern long long int curr_time;
extern priority_queue<vector<int>, vector<vector<int>>, Compare> sendingQueue;     // {timestamp, t(0)/b(1), ID, rcv}
extern priority_queue<vector<int>, vector<vector<int>>, Compare> transactionQueue; // {timestamp, sender}
extern priority_queue<vector<int>, vector<vector<int>>, Compare> blockQueue;       // {timestamp, block_ID, sender}

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

    Block(int miner_id, int parent_id, vector<int> txns)
    {
        this->BlkID = blkIDctr++;
        this->miner_id = miner_id;
        this->parent_id = parent_id;
        this->txns = txns;
    }
};

extern unordered_map<int, Block *> globalBlocks;             // maps block id to block ptr
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

class P2P;

class Peer
{
public:
    int peerID, mpeerID;
    bool slow;
    bool lowCPU;
    int maxDepth;
    double hash_power;
    double total_blocks;
    int total_transactions;
    int failed_txns;
    vector<int> neighbours;
    int longestChain;                          // block ID of leaf at end of longest chain
    set<int> memPool;                          // stores txn IDs
    set<int> blockSet;                         // stores IDs of blocks seen by peer
    set<int> leafBlocks;
    set<int> transactionSet;                   // stores IDs of transactions seen by peer
    treeNode *genesis_blk;                     // root of the tree
    map<int, treeNode *> blockTree;            // maps blockID to the corresponding treeNode
    set<int> orphanBlocks;                     // set of orphan blocks
    map<int, vector<pair<int, int>>> timeline; // maps time to the all block IDs that were received at that time
    map<int, vector<pair<int, int>>> valid_timeline; // maps time to the all valid block IDs that were received at that time
    P2P *NormNet, *MalNet;                            // pointer to global simulator

    Peer(int pID, int mID, P2P *NN, P2P* MN)
    {
        total_blocks = 0;
        total_transactions = 0;
        failed_txns = 0;
        maxDepth = 0;
        peerID = pID;
        mpeerID = mID;
        longestChain = 0;
        NormNet = NN;
        MalNet = MN;
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
    void createTree(Block*);
    bool query(int);
    void broadcastBlock(int);
    void processOrphanBlocks(Block &);
    bool validateBlock(Block &, map<int, int> &);
    void writeBlockTimesToFile();
    void addBlocktoTree(int);
    int blocks_in_longest_chain();
    void treeAnalysis();
};

class P2P // simulator class
{
public:
    int z0, z1, I, Ttx, simTime; // simulation parameters
    int numPeers;
    vector<Peer> peers;                // all peers in the simulation
    vector<vector<double>> link_speed; // link speeds of all peers
    vector<vector<double>> prop_delay; // prop delay of all peers
    int max_txn, max_block;
    int total_transactions;
    int forks;

    P2P(int z0, int z1, int np, int st, int I, int Ttx) : z0(z0), z1(z1), numPeers(np), I(I), Ttx(Ttx)
    {
        total_transactions = 0;
        forks = 0;
        max_txn = 0;
        max_block = 0;
        simTime = st;
        vector<vector<int>> graph = generate_graph(numPeers);
        for (int i = 0; i < numPeers; i++)
        {
            peers.push_back(Peer(i, this));
            peers[i].neighbours = graph[i];
        }
        assignPropDelay();
        assignLinkSpeed();
    }
    void assignPropDelay();
    void assignLinkSpeed();
    void start();
    int calculateLatency(int, int, double);
};

#endif