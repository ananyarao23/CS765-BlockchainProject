#include <cstdlib>
#include "helper.h"
#include "structure.h"
#include "network.h"

using namespace std;

/*The peer samples an interarrival period and adds the generation to the event queue*/
void Peer ::generateTransaction()
{
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
        // cout << "[HP]: Peer " << peerID << " failed to generate txn, balance is 0" << endl;
        failed_txns++;
        generateTransaction();
        return;
    }
    // simulator->total_transactions++;
    // srand(time(0));
    int amt = (rand() % balance) + 1;
    int rcv;
    do
    {
        rcv = (rand() % numPeers);
    } while (rcv == peerID);

    vector<int> txn_params = {txnIDctr++, peerID, rcv, amt};
    string new_txn = construct_txn(txn_params);
    cout<<"WWWWW"<<new_txn<<endl;
    memPool.insert(new_txn);
    transactionSet.insert(txn_params[0]);
    total_transactions++;

    // cout << "[HP]: " << peerID << " generated txn " << txn_params[0] << endl;

    for (auto p : neighbours)
    {   
        int latency = normNet->calculateLatency(peerID, p, 1);

        normNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, new_txn});
        // cout << "[HPH]: " << peerID << " sending txn " << txn_params[0] << " to " << p << endl;
    }
    generateTransaction();
}

void MaliciousPeer ::broadcastTransaction()
{
    int balance = blockTree.at(malicious_leaf)->balances[peerID];
    if (balance == 0)
    {
        // cout << "[MP]: Peer " << peerID << " failed to generate txn, balance is 0" << endl;
        failed_txns++;
        generateTransaction();
        return;
    }
    // simulator->total_transactions++;
    // srand(time(0));
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

    // cout << "[MP]: " << peerID << " generated txn " << txn_params[0] << endl;

    for (auto p : neighbours)
    {
        int latency = normNet->calculateLatency(peerID, p, 1);
        normNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, new_txn});
        // cout << "[MPH]: " << peerID << " sending txn " << txn_params[0] << " to " << p << endl;
    }

    for (auto p : malicious_neighbours)
    {
        int latency = malNet->calculateLatency(peerID, p, 1);
        malNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, new_txn});
        // cout << "[MPO]: " << peerID << "sending txn " << txn_params[0] << " to " << p << endl;
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
        // cout << "[HP]: " << peerID << " received txn " << txn_id << " from " << sender_id << endl;
        memPool.insert(txn);
        total_transactions++;
        for (auto p : neighbours)
        {
            if (p != sender_id)
            {
                int latency = normNet->calculateLatency(peerID, p, 1);
                if (query(p))
                {
                    // cout << "[HPH]: " << peerID << " sending txn " << txn_id << " to " << p << endl;
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
        // cout << "[MP]: "  << peerID << " received txn " << txn_id << " from " << sender_id << endl;
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
                    // cout << "[MPH]: "  << peerID << " sending txn " << txn_id << " to " << p << endl;
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
                    // cout << "[MPO]: "  << peerID << " sending txn " << txn_id << " to " << p << endl;
                    malNet->sendingQueue.push({{curr_time + latency, 0, peerID, p}, txn});
                }
            }
        }
    }
}