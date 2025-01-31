#include "simulator.h"
#include <random>
using namespace std;


std::vector<int> randomIndices(int x, int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, n);

    std::vector<int> indices;
    for (int i = 0; i < x; ++i) {
        indices.push_back(dist(gen));
    }

    return indices;
}

void P2P::assignSlowFast(int z0){
    int num = z0*NUM_PEERS/100;
    vector<int> indices = randomIndices(num,NUM_PEERS);
    for(int i=0;i<indices.size();i++){
        peers[indices[i]]->slow = true;
    }
}

void P2P::assignCPU(int z0){
    int num = z0*NUM_PEERS/100;
    vector<int> indices = randomIndices(num,NUM_PEERS);
    for(int i=0;i<indices.size();i++){
        peers[indices[i]]->lowCPU = true;
    }
}

void P2P::start(){
    pthread_t txn_threads[NUM_PEERS];
    for(int p=0;p<NUM_PEERS;p++){
        
        peers[p]->activateTransactions(txn_threads[p]);
    }
}