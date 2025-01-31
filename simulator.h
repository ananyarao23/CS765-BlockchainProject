#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <ctime>
#include <vector>

static int MAX_ID_PEER = 1;
static int NUM_PEERS;

class P2P{
    Peer** peers;
    P2P(int n){
        NUM_PEERS = n;
        peers = new Peer*[n];
    }

    void assignSlowFast(int z0);
    void assignCPU(int z0);
    void start();
};

class Peer{
private:
    int balance;
    static void* generateTransaction(void* arg){
        Peer* peer = static_cast<Peer*>(arg);
        std::srand(std::time(0));
        int IDy;
        do
        {
            IDy = 1 + rand() % NUM_PEERS;
        } while (IDy == peer->ID);
        int C = 1 + rand() % peer->balance;
    }

public:
    int ID;
    bool slow;
    bool lowCPU;

    Peer(){
        ID = MAX_ID_PEER++;
    }

    void activateTransactions(pthread_t txn_thread){
        if(pthread_create(&txn_thread, NULL, Peer::generateTransaction, this) != 0){
            perror("pthread_create");
        }
    }   
};

struct Transaction{
    int IDx,IDy;
    int C;
};
    
class Blocks{
    int ID;
    std::vector<Transaction> transactions;
    void addTransaction(Transaction txn){
        transactions.push_back(txn);
    }
};


