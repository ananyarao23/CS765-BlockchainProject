#ifndef SIMULATOR_H
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
#include <unordered_set>
#include "helper.h"
#include "network.h"
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

class Block
{
public:
    int BlkID;
    int miner_id;
    string parent_hash;
    vector<string> txns;

    Block(int miner_id, string parent_hash, vector<string> txns)
    {
        this->miner_id = miner_id;
        this->parent_hash = parent_hash;
        this->txns = txns;
        this->BlkID = blkIDctr++;
    }
};


class treeNode // for the nodes of the tree stored by each peer
{
    public:
    int depth;
    string parent_hash;
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
    bool slow;
    int maxDepth;
    double total_blocks;
    int total_transactions;
    int failed_txns;
    vector<int> neighbours;
    int longestChain;                          // block ID of leaf at end of longest chain
    set<string> memPool;                          // stores txn IDs
    set<int> blockSet;                         // stores IDs of blocks seen by peer
    set<int> leafBlocks;
    set<int> transactionSet;                   // stores IDs of transactions seen by peer
    treeNode *genesis_blk;                     // root of the tree
    map<string, treeNode *> blockTree;            // maps blockID to the corresponding treeNode
    set<string> orphanBlocks;                     // set of orphan blocks
    map<int, vector<pair<int, string>>> timeline; // maps time to the all block IDs that were received at that time
    map<int, vector<pair<int, string>>> valid_timeline; // maps time to the all valid block IDs that were received at that time                           // pointer to global simulator
    unordered_map<int, Transaction *> seen_txns; // maps transaction id to transaction ptr
    unordered_map<string, string> seen_blocks; //block hash to block content map
    unordered_map<string, queue<int>> pending_requests; //block hash to node ids that sent the hash map
    unordered_map<string, int> timeouts; //block hash to timeout counter map
    
    Peer() {}

    void setHashPower(double);
    void setlowCPU();
    void setslow();
    void generateTransaction();
    void broadcastTransaction();
    void receiveTransaction(int);
    void generateBlock();
    void receiveBlock(string, int, Block*);
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
    void receiveHash(string, int);
    virtual void broadcastHash(string) {}
    virtual void sendGetRequest(string, int) {}
    virtual void receiveGetRequest(string,int,int net = 0) {}
    virtual void sendBlock(string,int,int net = 0) {}
};

class MaliciousPeer : public Peer
{
    private:
    Network* normNet;
    OverlayNetwork* malNet;
    int peerID, malID;

    public:
    MaliciousPeer(int pID, int mID, Network* normNet, OverlayNetwork* malNet)
    {
        this->normNet = normNet;
        this->malNet = malNet;
        peerID = pID;
        malID = mID;
        total_blocks = 0;
        total_transactions = 0;
        failed_txns = 0;
        maxDepth = 0;
        longestChain = 0;
    }
};

class HonestPeer : public Peer
{
    private:
    Network* normNet;
    int peerID;

    public:
    HonestPeer(int pID, Network* normNet)
    {
        this->normNet = normNet;
        peerID = pID;
        total_blocks = 0;
        total_transactions = 0;
        failed_txns = 0;
        maxDepth = 0;
        longestChain = 0;
    }
};


#endif