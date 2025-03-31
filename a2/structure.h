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
extern bool eclipse_attack;

class Block
{
public:
    int BlkID;
    int miner_id;
    string parent_hash;
    vector<string> txns;

    Block(int id, int miner_id, string parent_hash, vector<string> txns);
};

class treeNode // for the nodes of the tree stored by each peer
{
public:
    int depth;
    string parent_hash;
    string hash;
    int block_id;
    int peerID;
    map<int, int> balances; // peerID to balances at this node

    treeNode(treeNode *parent_node, int id, int peerID);
};

class Network;
class OverlayNetwork;

class Peer
{
public:
    int peerID;
    Network *normNet;
    // int malicious_len;
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
    map<int, vector<pair<int, int>>> timeline;       // maps time to the all block IDs that were received at that time
    map<int, vector<vector<int>>> valid_timeline; // maps time to the all valid block IDs that were received at that time
    unordered_map<string, Block *> seen_blocks;            // block hash to block content map
    unordered_map<string, queue<int>> pending_requests;    // block hash to node ids that sent the hash map
    unordered_map<string, int> timeouts;                   // block hash to timeout counter map
    vector<string> toRemove;
    string longestMalChain;
    // string malicious_leaf;

    Peer();
    
    virtual ~Peer();

    void setHashPower(double);
    void generateTransaction();
    bool query(int);
    void broadcastBlock(int);
    bool validateBlock(Block *, map<int, int> &);
    void writeBlockTimesToFile();
    virtual pair<int, int> blocks_in_longest_chain() = 0;
    // void treeAnalysis();
    virtual void startReleasingChain() = 0;
    virtual void createTree(Block *) = 0;
    virtual void broadcastTransaction() = 0;
    virtual void receiveTransaction(string) = 0;
    virtual void generateBlock() = 0;
    virtual void receiveBlock(int, string) = 0;
    virtual void broadcastHash(string, int net = 0) = 0;
    virtual void receiveHash(string, int, int net = 0) = 0;
    virtual void sendGetRequest(string, int, int net = 0) = 0;
    virtual void receiveGetRequest(string, int, int net = 0) = 0;
    virtual void sendBlock(string, int, int net = 0) = 0;
    virtual void addBlocktoTree(string) = 0;
    virtual void releaseChain(string) = 0;
    virtual void processOrphanBlocks(string) = 0;
};

class MaliciousPeer : public Peer
{
private:
    OverlayNetwork *malNet;
    // string curr_attack_root;  // hash val
    set<string> released_chains;
    set<string> blocks_to_release;

public:
    MaliciousPeer(int pID, int mID, Network *normNet, OverlayNetwork *malNet);
    ~MaliciousPeer() {}
    pair<int, int> blocks_in_longest_chain() override;
    void startReleasingChain() override;
    void createTree(Block *) override;
    void receiveBlock(int, string) override;
    void generateBlock() override;
    void receiveHash(string, int, int net = 0) override;
    void broadcastHash(string, int net = 0) override;
    void sendGetRequest(string, int, int net = 0) override;
    void receiveGetRequest(string, int, int net = 1) override;
    void sendBlock(string, int, int net = 1) override;
    void addBlocktoTree(string) override;
    void receiveTransaction(string) override;
    void broadcastTransaction() override;
    void releaseChain(string) override;
    void processOrphanBlocks(string) override;
};

class HonestPeer : public Peer
{
public:
    HonestPeer(int pID, Network *normNet);
    ~HonestPeer() {}
    pair<int, int> blocks_in_longest_chain() override;
    void startReleasingChain() override;
    void createTree(Block *) override;
    void receiveBlock(int, string) override;
    void receiveHash(string, int, int net = 0) override;
    void broadcastHash(string, int net = 0) override;
    void sendGetRequest(string, int, int net = 0) override;
    void receiveGetRequest(string, int, int net = 0) override;
    void sendBlock(string, int, int net = 0) override;
    void addBlocktoTree(string) override;
    void generateBlock() override;
    void receiveTransaction(string) override;
    void broadcastTransaction() override;
    void releaseChain(string) override;
    void processOrphanBlocks(string) override;
};

#endif