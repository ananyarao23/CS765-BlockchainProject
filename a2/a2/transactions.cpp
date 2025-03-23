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

    vector<int> txn_params = {txnIDctr++, peerID, rcv, amt};
    string new_txn = construct_txn(txn_params);
    memPool.insert(txn_params[0]);
    transactionSet.insert(txn_params[0]);
    total_transactions++;

    for (auto p : neighbours)
    {
        int latency = simulator->calculateLatency(peerID, p, 8);

        sendingQueue.push({{curr_time + latency, 0, p}, new_txn});
    }
    generateTransaction();
}

/* txn_id is the ID of the transaction received from another peer on the network.
 The peer forwards it to its neighbours that haven't seen it yet */
void Peer ::receiveTransaction(string txn)
{
    vector<int> txn_vector = parse_txn(txn);
    int txn_id = txn_vector[0];
    int sender_id = txn_vector[1];
    int receiver_id = txn_vector[2];
    int amount = txn_vector[3];

    if (transactionSet.find(txn_id) == transactionSet.end())
    {
        memPool.insert(txn_id);
        total_transactions++;
        for (auto p : neighbours)
        {
            if (p != sender_id)
            {
                int latency = simulator->calculateLatency(peerID, p, 8);
                if (query(p))
                    sendingQueue.push({{curr_time + latency, 0, p}, txn});
            }
        }
    }
}