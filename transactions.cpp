#include "simulator.h"
#include <random>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>

void Peer ::generateTransaction()
{
    int timestamp = curr_time + generateExponential(Ttx);
    transactionQueue.push({timestamp, peerID});
}

// we've popped an element from transactionQueue, now need to complete and push to sending queue
void Peer ::broadcastTransaction()
{
    // coordinate with treenodebalances
    int balance = ;
    srand(time(0));
    int amt = (rand() % balance) + 1;
    int rcv = (rand() % num_peers);
    Transaction *new_txn = new Transaction(peerID, rcv, amt);
    globalTransactions[new_txn->txID] = new_txn;
    for (auto p : neighbours)
    {
        int latency = calculateLatency(peerID, p, 0.008);
        sendingQueue.push({curr_time + latency, 0, new_txn->txID, p});
    }
    generateTransaction();
}

// we've popped an element from sendingQueue and checked that it's a txn, now need to validate and fwd
void Peer ::receiveTransaction(int txn_id)
{
    if (transactionSet.find(txn_id) == transactionSet.end())
    {
        transactionSet.insert(txn_id);
        memPool.insert(txn_id);
        for (auto p : neighbours)
        {
            int latency = calculateLatency(peerID, p, 0.008);
            sendingQueue.push({curr_time + latency, 0, txn_id, p});
        }
    }
}