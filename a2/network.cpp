#include "network.h"
#include <fstream>
#include <iomanip>

using namespace std;

/* Samples propagation delays from a uniform distribution */
void P2P::assignPropDelay()
{
    for (int i = 0; i < numPeers; i++)
    {
        prop_delay.push_back(vector<double>(numPeers));
        for (int j = 0; j < numPeers; j++)
        {
            prop_delay[i][j] = sampleUniform(0.01, 0.5);
        }
    }
}

/* Assign link speeds based on both nodes */
void P2P::assignLinkSpeed()
{
    for (int i = 0; i < numPeers; i++)
    {
        link_speed.push_back(vector<double>(numPeers));
        for (int j = 0; j < numPeers; j++)
        {
            link_speed[i][j] = peers[i]->slow || peers[j]->slow ? 5 : 100;
        }
    }
}

/* Returns the latency (in ms) gives peer IDs and message size */
int P2P::calculateLatency(int i, int j, double ms)
{
    double p = prop_delay[i][j] * 1000;  // ms
    double c = link_speed[i][j];         // Mbps
    int d = generateExponential(96 / c); // ms
    // ms in kilobits
    int latency = p + d + ms / c;
    return latency;
}

void Network::run(long long curr_time)
{
    // cout << "Running sim for honest network" << endl;
    vector<int> next_txn;
    pair<vector<int>, string> next_msg, next_tout, next_blk;

    if (!blockQueue.empty())
    {
        next_blk = blockQueue.top();
    }

    while (!blockQueue.empty() && next_blk.first[0] == curr_time)

    { // a block is ready to be broadcasted by its miner
        blockQueue.pop();
        int sender = next_blk.first[1];
        string hash = next_blk.second;
        if (peers[sender]->longestChain == peers[sender]->seen_blocks[hash]->parent_hash)
        {
            peers[sender]->total_blocks++;
            peers[sender]->addBlocktoTree(hash);
            peers[sender]->timeline[curr_time].push_back({hash, peers[sender]->seen_blocks[hash]->parent_hash});
            peers[sender]->valid_timeline[curr_time].push_back({hash, peers[sender]->seen_blocks[hash]->parent_hash});
            peers[sender]->broadcastHash(hash);
            peers[sender]->generateBlock();
        }
        else
        {
            peers[sender]->seen_blocks.erase(hash);
        }
        if (!blockQueue.empty())
            next_blk = blockQueue.top();
    }
    if (!transactionQueue.empty())
    {
        next_txn = transactionQueue.top();
    }
    while (!transactionQueue.empty() && next_txn[0] == curr_time)

    {
        // a transaction is ready to be broadcasted by its creator
        total_transactions++;
        transactionQueue.pop();
        peers[next_txn[1]]->broadcastTransaction();
        if (!transactionQueue.empty())
            next_txn = transactionQueue.top();
    }
    if (!sendingQueue.empty())
    {
        next_msg = sendingQueue.top();
    }
    while (!sendingQueue.empty() && next_msg.first[0] == curr_time)

    { // a message (block or transaction) is ready to be received by a peer (targeted receiver)
        sendingQueue.pop();
        if (next_msg.first[1] == 0) // txn
        {
            peers[next_msg.first[3]]->receiveTransaction(next_msg.second);
        }
        else if (next_msg.first[1] == 1) // block
        {
            peers[next_msg.first[3]]->receiveBlock(next_msg.first[2], next_msg.second);
        }
        else if (next_msg.first[1] == 2) // hash
        {
            peers[next_msg.first[3]]->receiveHash(next_msg.second, next_msg.first[2]);
        }
        else if (next_msg.first[1] == 3) // get
        {
            peers[next_msg.first[3]]->receiveGetRequest(next_msg.second, next_msg.first[2]);
        }
        if (!sendingQueue.empty())
        {
            next_msg = sendingQueue.top();
        }
    }

    if (!timeoutQueue.empty())
    {
        next_tout = timeoutQueue.top();
    }

    while (!timeoutQueue.empty() && next_tout.first[0] == curr_time)
    {
        // timeout expired
        timeoutQueue.pop();
        int peerID = next_tout.first[1];
        string hash = next_tout.second;
        // it's possible that block has already been received, and data structures have been cleared
        if (peers[peerID]->pending_requests.find(hash) == peers[peerID]->pending_requests.end())
        {
            continue;
        }
        peers[peerID]->pending_requests[hash].pop();
        if (!peers[peerID]->pending_requests[hash].empty())
        {
            int sender_id = peers[peerID]->pending_requests[hash].front();
            peers[peerID]->timeouts[hash] = curr_time + Tt;
            timeoutQueue.push({{curr_time + Tt, peerID}, hash});
            peers[peerID]->sendGetRequest(hash, sender_id);
        }

        if (!timeoutQueue.empty())
        {
            next_tout = timeoutQueue.top();
        }
    }
}

void OverlayNetwork::run(long long curr_time)
{
    // cout << "Running sim for overlay network" << endl;
    vector<int> next_txn;
    pair<vector<int>, string> next_blk;
    pair<vector<int>, string> next_msg, next_tout;

    if (!blockQueue.empty())
    {
        next_blk = blockQueue.top();
    }

    while (!blockQueue.empty() && next_blk.first[0] == curr_time)

    { // a block is ready to be broadcasted by its miner

        blockQueue.pop();
        int sender = next_blk.first[1];
        string blk_hash = next_blk.second;
        if (peers[sender]->malicious_leaf == peers[sender]->seen_blocks[blk_hash]->parent_hash)
        {
            peers[sender]->total_blocks++;
            peers[sender]->addBlocktoTree(blk_hash);
            peers[sender]->timeline[curr_time].push_back({blk_hash, peers[sender]->seen_blocks[blk_hash]->parent_hash});
            peers[sender]->valid_timeline[curr_time].push_back({blk_hash, peers[sender]->seen_blocks[blk_hash]->parent_hash});
            peers[sender]->broadcastHash(blk_hash);
            peers[sender]->generateBlock();
        }
        else
        {
            peers[sender]->seen_blocks.erase(blk_hash);
        }
        if (!blockQueue.empty())
            next_blk = blockQueue.top();
    }
    if (!transactionQueue.empty())
    {
        next_txn = transactionQueue.top();
    }
    while (!transactionQueue.empty() && next_txn[0] == curr_time)

    {
        // a transaction is ready to be broadcasted by its creator
        total_transactions++;
        transactionQueue.pop();
        peers[next_txn[1]]->broadcastTransaction();
        if (!transactionQueue.empty())
            next_txn = transactionQueue.top();
    }
    if (!sendingQueue.empty())
    {
        next_msg = sendingQueue.top();
    }
    while (!sendingQueue.empty() && next_msg.first[0] == curr_time)

    { // a message (block or transaction) is ready to be received by a peer (targeted receiver)
        sendingQueue.pop();
        if (next_msg.first[1] == 0) // txn
        {
            peers[next_msg.first[3]]->receiveTransaction(next_msg.second);
        }
        else if (next_msg.first[1] == 1) // block
        {
            peers[next_msg.first[3]]->receiveBlock(next_msg.first[2], next_msg.second);
        }
        else if (next_msg.first[1] == 2) // hash
        {
            peers[next_msg.first[3]]->receiveHash(next_msg.second, next_msg.first[2], 1);
        }
        else if (next_msg.first[1] == 3) // get
        {
            peers[next_msg.first[3]]->receiveGetRequest(next_msg.second, next_msg.first[2], 1);
        }
        if (!sendingQueue.empty())
        {
            next_msg = sendingQueue.top();
        }
    }

    if (!timeoutQueue.empty())
    {
        next_tout = timeoutQueue.top();
    }

    while (!timeoutQueue.empty() && next_tout.first[0] == curr_time)
    {
        // timeout expired
        timeoutQueue.pop();
        int peerID = next_tout.first[1];
        string hash = next_tout.second;
        // it's possible that block has already been received, and data structures have been cleared
        if (peers[peerID]->pending_requests.find(hash) == peers[peerID]->pending_requests.end())
        {
            continue;
        }
        peers[peerID]->pending_requests[hash].pop();
        if (!peers[peerID]->pending_requests[hash].empty())
        {
            int sender_id = peers[peerID]->pending_requests[hash].front();
            peers[peerID]->timeouts[hash] = curr_time + Tt;
            timeoutQueue.push({{curr_time + Tt, peerID}, hash});
            peers[peerID]->sendGetRequest(hash, sender_id);
        }

        if (!timeoutQueue.empty())
        {
            next_tout = timeoutQueue.top();
        }
    }
}
