#ifndef STRUCTURE_H
#define STRUCTURE_H

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
#include <string>
#include "helper.h"
using namespace std;

extern int txnIDctr; // for setting txn ID
extern int blkIDctr; // for setting block ID
extern long long int curr_time;
extern float Tt;
extern float Ttx;
extern float I;
extern int numPeers;

class Block
{
public:
    int BlkID;
    int miner_id;
    string parent_hash;
    vector<string> txns;

    Block(int id, int miner_id, string parent_hash, vector<string> txns)
    {
        this->miner_id = miner_id;
        this->parent_hash = parent_hash;
        this->txns = txns;
        this->BlkID = id;
    }
};

class treeNode // for the nodes of the tree stored by each peer
{
public:
    int depth;
    string parent_hash;
    string hash;
    int block_id;
    map<int, int> balances; // peerID to balances at this node

    treeNode(treeNode *parent_node, int id)
    {
        this->depth = parent_node ? parent_node->depth + 1 : 0;
        this->parent_hash = parent_node ? parent_node->parent_hash: "";
        this->block_id = id;
    }
};

class Network;
class OverlayNetwork;

class Peer
{
public:
    int peerID;
    Network *normNet;
    int malicious_len;
    set<string> memPool2;
    bool slow;
    double hash_power;
    int maxDepth;
    double total_blocks;
    int total_transactions;
    int failed_txns;
    vector<int> neighbours;
    vector<int> malicious_neighbours;
    string longestChain; // block ID of leaf at end of longest chain
    set<string> memPool; // stores txn IDs
    set<string> leafBlocks;
    set<int> transactionSet;                               // stores IDs of transactions seen by peer
    treeNode *genesis_blk;                                 // root of the tree
    map<string, treeNode *> blockTree;                     // maps blockID to the corresponding treeNode
    set<string> orphanBlocks;                              // set of orphan blocks
    map<int, vector<pair<string, string>>> timeline;       // maps time to the all block IDs that were received at that time
    map<int, vector<pair<string, string>>> valid_timeline; // maps time to the all valid block IDs that were received at that time
    unordered_map<string, Block *> seen_blocks;            // block hash to block content map
    unordered_map<string, queue<int>> pending_requests;    // block hash to node ids that sent the hash map
    unordered_map<string, int> timeouts;                   // block hash to timeout counter map
    vector<string> toRemove;
    string malicious_leaf;

    Peer()
    {
        malicious_len = 0;
        malicious_neighbours = {};
    }
    virtual ~Peer();

    void setHashPower(double);
    void generateTransaction();
    void createTree(Block *);
    bool query(int);
    void broadcastBlock(int);
    void processOrphanBlocks(string);
    bool validateBlock(Block *, map<int, int> &);
    // void writeBlockTimesToFile();
    int blocks_in_longest_chain();
    // void treeAnalysis();
    virtual void broadcastTransaction() = 0;
    virtual void receiveTransaction(string) = 0;
    virtual void generateBlock() = 0;
    virtual void receiveBlock(int, string) = 0;
    virtual void broadcastHash(string) = 0;
    virtual void receiveHash(string, int, int net = 0) = 0;
    virtual void sendGetRequest(string, int, int net = 0) = 0;
    virtual void receiveGetRequest(string, int, int net = 0) = 0;
    virtual void sendBlock(string, int, int net = 0) = 0;
    virtual void addBlocktoTree(string) = 0;
};

class MaliciousPeer : public Peer
{
private:
    OverlayNetwork *malNet;

public:
    MaliciousPeer(int pID, int mID, Network *normNet, OverlayNetwork *malNet)
    {
        this->normNet = normNet;
        this->malNet = malNet;
        peerID = pID;
        total_blocks = 0;
        total_transactions = 0;
        failed_txns = 0;
        maxDepth = 0;
        longestChain = "";
        slow = false;
    }
    ~MaliciousPeer() {}
    void receiveBlock(int, string) override;
    void generateBlock() override;
    void receiveHash(string, int, int net = 0) override;
    void broadcastHash(string) override;
    void sendGetRequest(string, int, int net = 0) override;
    void receiveGetRequest(string, int, int net = 1) override;
    void sendBlock(string, int, int net = 1) override;
    void addBlocktoTree(string) override;
    void receiveTransaction(string) override;
    void broadcastTransaction() override;
};

class HonestPeer : public Peer
{
public:
    HonestPeer(int pID, Network *normNet)
    {
        this->normNet = normNet;
        peerID = pID;
        total_blocks = 0;
        total_transactions = 0;
        failed_txns = 0;
        maxDepth = 0;
        longestChain = "";
        slow = true;
    }
    ~HonestPeer() {}
    void receiveBlock(int, string) override;
    void receiveHash(string, int, int net = 0) override;
    void broadcastHash(string) override;
    void sendGetRequest(string, int, int net = 0) override;
    void receiveGetRequest(string, int, int net = 0) override;
    void sendBlock(string, int, int net = 0) override;
    void addBlocktoTree(string) override;
    void generateBlock() override;
    void receiveTransaction(string) override;
    void broadcastTransaction() override;
};

#endif