#include "structure.h"
#include <fstream>
#include <iomanip>
#include <filesystem>

using namespace std;

int num_txns = 0;
int time_stamp = 0;

int txnIDctr = 0; // for setting txn ID
int blkIDctr = 1; // for setting block ID

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
            link_speed[i][j] = peers[i].slow || peers[j].slow ? 5 : 100;
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

/* Starts the simulation */
void P2P::start()
{
    Block *genesis_block = new Block(0, -1, {});
    // globalBlocks[genesis_block->BlkID] = genesis_block;
    for (int i = 0; i < numPeers; i++)
    {
        peers[i].createTree(genesis_block);
        peers[i].generateBlock();
        peers[i].generateTransaction();
    }
}

void P2P::run(long long curr_time)
{
    vector<int> next_blk, next_txn;
    pair<vector<int>, string> next_msg, next_tout;

    if (!blockQueue.empty())
    {
        next_blk = blockQueue.top();
    }

    while (!blockQueue.empty() && next_blk[0] == curr_time)

    { // a block is ready to be broadcasted by its miner
        blockQueue.pop();
        int sender = next_blk[2];
        int blkid = next_blk[1];
        if (peers[sender].longestChain == globalBlocks[blkid]->parent_id)
        {
            peers[sender].total_blocks++;
            peers[sender].addBlocktoTree(blkid);
            peers[sender].timeline[curr_time].push_back({blkid, globalBlocks[blkid]->parent_id});
            peers[sender].valid_timeline[curr_time].push_back({blkid, globalBlocks[blkid]->parent_id});
            peers[sender].broadcastBlock(blkid);
            peers[sender].generateBlock();
        }
        else
        {
            delete globalBlocks[blkid];
            globalBlocks[blkid] = NULL;
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
        peers[next_txn[1]].broadcastTransaction();
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
            peers[next_msg.first[2]].receiveTransaction(next_msg.second);
        }
        else if (next_msg.first[1] == 1) // block
        {
            peers[next_msg.first[1]].receiveBlock(next_msg.second);
        }
        else if (next_msg.first[1] == 2) // hash
        {
        }
        else if (next_msg.first[1] == 3) // get
        {
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
        peers[peerID].pending_requests[hash].pop();
        if (!peers[peerID].pending_requests[hash].empty())
        {
            int sender_id = peers[peerID].pending_requests[hash].front();
            peers[peerID].timeouts[hash] = curr_time + Tt;
            timeoutQueue.push({{curr_time + Tt, peerID}, hash});
            peers[peerID].sendGetRequest(hash, sender_id);
        }

        if (!timeoutQueue.empty())
        {
            next_tout = timeoutQueue.top();
        }
    }
}
