#include "structure.h"
#include <fstream>
#include <iomanip>
#include <filesystem>

using namespace std;

int num_txns = 0;
int time_stamp = 0;

int txnIDctr = 0; // for setting txn ID
int blkIDctr = 1; // for setting block ID
// long long int curr_time = 0; // time counter
priority_queue<vector<int>, vector<vector<int>>, Compare> sendingQueue;
priority_queue<vector<int>, vector<vector<int>>, Compare> transactionQueue;
priority_queue<vector<int>, vector<vector<int>>, Compare> blockQueue;

// unordered_map<int, Block *> globalBlocks;             // maps block ID to block ptr
unordered_map<int, Transaction *> globalTransactions; // maps transaction ID to transaction ptr


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

    // // Discrete event simulator
    // while (curr_time < simTime)
    // {
    //     vector<int> next_blk, next_msg, next_txn;

    //     if (!blockQueue.empty())
    //     {
    //         next_blk = blockQueue.top();
    //     }

    //     while (!blockQueue.empty() && next_blk[0] == curr_time)

    //     { // a block is ready to be broadcasted by its miner
    //         blockQueue.pop();
    //         int sender = next_blk[2];
    //         int blkid = next_blk[1];
    //         if (peers[sender].longestChain == globalBlocks[blkid]->parent_id)
    //         {
    //             peers[sender].total_blocks++;
    //             peers[sender].addBlocktoTree(blkid);
    //             peers[sender].timeline[curr_time].push_back({blkid, globalBlocks[blkid]->parent_id});
    //             peers[sender].valid_timeline[curr_time].push_back({blkid, globalBlocks[blkid]->parent_id});
    //             peers[sender].broadcastBlock(blkid);
    //             peers[sender].generateBlock();
    //         }
    //         else
    //         {
    //             delete globalBlocks[blkid];
    //             globalBlocks[blkid] = NULL;
    //         }
    //         if (!blockQueue.empty())
    //             next_blk = blockQueue.top();
    //     }
    //     if (!transactionQueue.empty())
    //     {
    //         next_txn = transactionQueue.top();
    //     }
    //     while (!transactionQueue.empty() && next_txn[0] == curr_time)

    //     {
    //         // a transaction is ready to be broadcasted by its creator
    //         total_transactions++;
    //         transactionQueue.pop();
    //         peers[next_txn[1]].broadcastTransaction();
    //         if (!transactionQueue.empty())
    //             next_txn = transactionQueue.top();
    //     }
    //     if (!sendingQueue.empty())
    //     {
    //         next_msg = sendingQueue.top();
    //     }
    //     while (!sendingQueue.empty() && next_msg[0] == curr_time)

    //     { // a message (block or transaction) is ready to be received by a peer (targeted receiver)
    //         sendingQueue.pop();
    //         if (next_msg[1] == 0)
    //         {
    //             peers[next_msg[3]].receiveTransaction(next_msg[; 2])
    //         }
    //         else
    //         {
    //             peers[next_msg[3]].receiveBlock(next_msg[2]);
    //         }
    //         if (!sendingQueue.empty())
    //             next_msg = sendingQueue.top();
    //     }
    //     curr_time++;
    // }
}


void P2P::run()
{
        vector<int> next_blk, next_msg, next_txn;

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
        while (!sendingQueue.empty() && next_msg[0] == curr_time)

        { // a message (block or transaction) is ready to be received by a peer (targeted receiver)
            sendingQueue.pop();
            if (next_msg[1] == 0)
            {
                peers[next_msg[3]].receiveTransaction(next_msg[; 2])
            }
            else
            {
                peers[next_msg[3]].receiveBlock(next_msg[2]);
            }
            if (!sendingQueue.empty())
                next_msg = sendingQueue.top();
        }
}
