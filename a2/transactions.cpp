#include "structure.h"
#include <cstdlib>

using namespace std;

/*The peer samples an interarrival period and adds the generation to the event queue*/
void Peer ::generateTransaction()
{
    int ts;
    do
    {
        ts = generateExponential(simulator->Ttx);
    } while (curr_time + ts < 0);

    transactionQueue.push({curr_time + ts, peerID});
}

/* After an interarrival period, the node generating the
transaction chooses a random destination node and txn amount*/
void Peer ::broadcastTransaction()
{
    int balance = blockTree[longestChain]->balances[peerID];
    if (balance == 0)
    {
        failed_txns++;
        generateTransaction();
        return;
    }
    // simulator->total_transactions++;
    srand(time(0));
    int amt = (rand() % balance) + 1;
    int rcv;
    do
    {
        rcv = (rand() % simulator->numPeers);
    } while (rcv == peerID);

    Transaction *new_txn = new Transaction(peerID, rcv, amt);
    globalTransactions[new_txn->txID] = new_txn;
    for (auto p : neighbours)
    {
        int latency = simulator->calculateLatency(peerID, p, 8);

        sendingQueue.push({curr_time + latency, 0, new_txn->txID, p});
    }
    generateTransaction();
}

/* txn_id is the ID of the transaction received from another peer on the network.
 The peer forwards it to its neighbours that haven't seen it yet */
void Peer ::receiveTransaction(int txn_id)
{
    if (transactionSet.find(txn_id) == transactionSet.end())
    {
        memPool.insert(txn_id);
        total_transactions++;
        for (auto p : neighbours)
        {
            if (p != globalTransactions[txn_id]->sender_id)
            {
                int latency = simulator->calculateLatency(peerID, p, 8);
                if (query(p))
                    sendingQueue.push({curr_time + latency, 0, txn_id, p});
            }
        }
    }
}