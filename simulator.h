#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <queue>
#include <random>

#include "random_graph.h"
using namespace std;

unordered_map<int,Block*> globalBlocks;
unordered_map<int,Transaction*> globalTransactions;


class Transaction{
    public:
    int amount ;
    int sender_id;
    int receiver_id;
    int txID;
    time_t timestamp;

    Transaction(int amount,int sender_id,int receiver_id,int txID){
        this->amount= amount;
        this->sender_id = sender_id;
        this->receiver_id = receiver_id;
        this->txID = txID;
        this->timestamp = time(0);
    }
};

class Block{
    public:
    int BlkID;
    int parent_id;
    vector<int> transactions;

    Block(int BlkID,int parent_id):BlkID(BlkID),parent_id(parent_id){}
};

class blockMetaData{
public:
    int depth;
    int parent_id;
    unordered_map<int,int> UTXOs;
    
    blockMetaData(int depth,int parent_id):depth(depth),parent_id(parent_id){}
};

class Peer : public P2P{
    private:
    int longestChain;
    int blockchainLeaf;
    int peerID;
    double hash_power;
    bool slow;
    bool lowCPU;
    bool coinbase;
    vector<int> transactionPool; // transactions pool (stored ids of transactions)
    unordered_map<int,bool> transactionRecord; // true if the transaction is already seen by the peer
    unordered_map<int,bool> blockRecord; // true if the block is already seen by the peer
    vector<int> leafBlocks; // stores ids of leaf blocks
    unordered_map<int,blockMetaData*> blockInfo; // maps all the blocks to their corresponding metadata class
    
    public:
    bool lowCPU;
    vector<int> neighbours;
    queue<int> transactionQueue;
    queue<int> blockQueue;

    Peer(int pID)
    {
        peerID = pID;
        coinbase = false;
    }
    void setHashPower(double);
    void setlowCPU();
    void setslow();
    void generateTransaction();
    void receiveTransaction();    
    void generateBlock();
    void receiveBlock();
    bool verifyBlock(int);
};

void Peer::setHashPower(double x){
    hash_power = x;
}

void Peer::setlowCPU(){
    lowCPU = true;
}

void Peer::setslow(){
    slow = true;
}

class P2P{
    private:
    int z0, z1;
    public:
    vector<Peer> peers;
    int numPeers;
    int max_txn,max_block;
    int Ttx;
    P2P(); // default constructor
    P2P(int z0,int z1,int np, int max_txn, int max_block): z0(z0), z1(z1), numPeers(np), max_txn(max_txn), max_block(max_block){
        vector<vector<int>> graph = generate_graph(numPeers);
        for(int i = 0; i < numPeers; i++){
            peers.push_back(Peer(i));
            peers[i].neighbours = graph[i];
        }
    }
    void assignSlowFast();
    void assignCPU();
    void computeHashPower();
    void start();

};

vector<int> randomIndices(int x, int n) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, n-1);

    vector<int> indices;
    for (int i = 0; i < x; ++i) {
        indices.push_back(dist(gen));
    }

    return indices;
}

void P2P::assignSlowFast(){
    int num = z0*numPeers/100;
    vector<int> indices = randomIndices(num,numPeers);
    for(int i : indices){
        peers[i].setslow();
    }
}

void P2P::assignCPU(){
    int num = z1*numPeers/100;
    vector<int> indices = randomIndices(num,numPeers);
    for(int i : indices){
        peers[i].setlowCPU();
    }
}


void P2P::computeHashPower(){
    double x;
    double coefficient = 0;
    for(int i=0;i<numPeers;i++){
       if (peers[i].lowCPU) coefficient += 1;
       else coefficient += 10;
    }
    x = 1.0/coefficient;
    for(int i=0;i<numPeers;i++){
       if (peers[i].lowCPU) peers[i].setHashPower(x);
       else peers[i].setHashPower(x*10);
    }
}





