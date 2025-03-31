#include <cstdlib>
#include "helper.h"
#include "structure.h"
#include "network.h"

using namespace std;

/*The peer samples an interarrival period and adds the generation to the event queue*/
void Peer ::generateTransaction()
{
    if (normNet->stop)
        return;
    int ts;
    do
    {
        ts = generateExponential(Ttx);
    } while (curr_time + ts < 0);

    normNet->transactionQueue.push({curr_time + ts, peerID});
}

/* After an interarrival period, the node generating the
transaction chooses a random destination node and txn amount*/
void HonestPeer ::broadcastTransaction()
{
    int balance = blockTree.at(longestChain)->balances[peerID];
    if (balance == 0)
    {
        failed_txns++;
        generateTransaction();
        return;
    }
    int amt = (rand() % balance) + 1;
    int rcv;
    do
    {
        rcv = (rand() % numPeers);
    } while (rcv == peerID);

    vector<int> txn_params = {txnIDctr++, peerID, rcv, amt};
    string new_txn = construct_txn(txn_params);
    memPool.insert(new_txn);
    transactionSet.insert(txn_params[0]);
    total_transactions++;


    for (auto p : neighbours)
    {   
        int latency = normNet->calculateLatency(peerID, p, 1);

        normNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, new_txn});
    }
    generateTransaction();
}

void MaliciousPeer ::broadcastTransaction()
{
    int balance = blockTree.at(longestMalChain)->balances[peerID];
    if (balance == 0)
    {
        failed_txns++;
        generateTransaction();
        return;
    }
    int amt = (rand() % balance) + 1;
    int rcv;
    do
    {
        rcv = (rand() % numPeers);
    } while (rcv == peerID);

    vector<int> txn_params = {txnIDctr++, peerID, rcv, amt};
    string new_txn = construct_txn(txn_params);
    memPool.insert(new_txn);
    memPool2.insert(new_txn);
    transactionSet.insert(txn_params[0]);
    total_transactions++;


    for (auto p : neighbours)
    {
        int latency = normNet->calculateLatency(peerID, p, 1);
        normNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, new_txn});
    }

    for (auto p : malicious_neighbours)
    {
        int latency = malNet->calculateLatency(peerID, p, 1);
        malNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, new_txn});
    }
    generateTransaction();
}

/* txn_id is the ID of the transaction received from another peer on the network.
 The peer forwards it to its neighbours that haven't seen it yet */
void HonestPeer::receiveTransaction(string txn)
{
    vector<int> txn_vector = parse_txn(txn);
    int txn_id = txn_vector[0];
    int sender_id = txn_vector[1];
    int receiver_id = txn_vector[2];
    int amount = txn_vector[3];

    if (transactionSet.find(txn_id) == transactionSet.end())
    {
        memPool.insert(txn);
        total_transactions++;
        for (auto p : neighbours)
        {
            if (p != sender_id)
            {
                int latency = normNet->calculateLatency(peerID, p, 1);
                if (query(p))
                {
                    normNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, txn});
                }
            }
        }
    }
}

void MaliciousPeer::receiveTransaction(string txn)
{
    vector<int> txn_vector = parse_txn(txn);
    int txn_id = txn_vector[0];
    int sender_id = txn_vector[1];
    int receiver_id = txn_vector[2];
    int amount = txn_vector[3];

    if (transactionSet.find(txn_id) == transactionSet.end())
    {
        memPool.insert(txn);
        memPool2.insert(txn);
        total_transactions++;
        for (auto p : neighbours)
        {
            if (p != sender_id)
            {
                int latency = normNet->calculateLatency(peerID, p, 1);
                if (query(p))
                {
                    normNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, txn});
                }
            }
        }

        for (auto p : malicious_neighbours)
        {
            if (p != sender_id)
            {
                int latency = malNet->calculateLatency(peerID, p, 1);
                if (query(p))
                {
                    malNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, txn});
                }
            }
        }
    }
}