#include "simulator.h"
#include <random>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>

// int I; // global I

double calculateBlockSize(Block *blk)
{
    return (1 + blk->txns.size()) * 8.0 / 1000.0;
}

void Peer::generateBlock()
{
    int num_txns = 0;
    vector<int> txns;
    for (auto e : memPool)
    {
        txns.push_back(e);
        num_txns++;
        if (num_txns > 999)
            break;
    }
    Block *blk = new Block(peerID, longestChain, txns);

    globalBlocks[blk->BlkID] = blk;

    int ts = generateExponential(I / hash_power);
    blockQueue.push({curr_time + ts, blk->BlkID, peerID});
}

void Peer::broadcastBlock(int blkid)
{
    double messagesize = calculateBlockSize(globalBlocks[blkid]);
    // remove txns from pool
    for (auto tid : globalBlocks[blkid]->txns)
    {
        memPool.erase(tid);
    }
    for (auto receiver:neighbours)
    {
        int lt = calculateLatency(peerID, receiver, messagesize);
        sendingQueue.push({curr_time + lt, 1, blkid, receiver});
    }
}
