#include "simulator.h"
#include <cstdlib>
#include <thread>
#include <chrono>



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

    int ts = generateExponential(I/ hash_power);
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

void Peer::receiveBlock(int blkid)
{
    Block *block = globalBlocks[blkid];
    //check if receiving peer has already seen the block
    if (blockSet.find(blkid) == blockSet.end())
    {
        blockSet.insert(blkid);

        if (blockTree.find(block->parent_id) != blockTree.end())
        {
            //now validate transactions in the block
            map<int, int> balances_temp;
            if (validateBlock(*block, balances_temp))
            {
                treeNode *parentNode = blockTree[block->parent_id];
                treeNode *child = new treeNode(parentNode, block);
                child->balances = balances_temp;
                blockTree[block->BlkID] = child;

                leafBlocks.erase(block->parent_id);
                leafBlocks.insert(block->BlkID);

                if (child->depth > maxDepth)
                {
                    maxDepth = child->depth;
                    longestChain = blkid;

                    for (int txn : block->txns)
                    {
                        memPool.erase(txn);
                    }
                }
                processOrphanBlocks(*block);
                broadcastBlock(block->BlkID);
            }
        }
        else
        {
            orphanBlocks.insert(blkid);
        }
    }
}

void Peer::processOrphanBlocks(Block& block) {
    // Flag to track if any orphan blocks were processed
    bool processedAnyOrphan = false;

    // Process the orphaned blocks
    for (int orphan : orphanBlocks) {
        if (blockTree[orphan]->parent_ptr->block_id == block.BlkID) {
            processedAnyOrphan = true;
            map<int, int> balances_temp;
            if (validateBlock(globalBlocks[blockTree[orphan]->block_id], balances_temp)) {
                treeNode* parentNode = blockTree[block.BlkID];
                treeNode* orphanChild = new treeNode(parentNode, globalBlocks[blockTree[orphan]->block_id]);
                orphanChild->balances = blockTree[orphan]->balances;
                blockTree[orphan] = orphanChild;
                leafBlocks.erase(blockTree[orphan]->parent_ptr->block_id);
                leafBlocks.insert(orphan);
                orphanBlocks.erase(orphan);

                // Recursively process any orphan blocks that are now children of the un-orphaned block
                processOrphanBlocks(globalBlocks[blockTree[orphan]->block_id]);
            }
        }
    }

    if (!processedAnyOrphan) {
        return;
    }
}


bool Peer::validateBlock(Block& block, map<int, int>& balances_temp){
    //to check go the chain in the tree which has the parent then at parent we have balances of each peer stored, validate each transaction
    treeNode* parent = blockTree[block.parent_id];
    map <int, int> balances = parent->balances;
    for(int txn : block.txns){
        Transaction* tx = globalTransactions[txn];
        if(balances[tx->sender_id] >= tx->amount){
            balances[tx->sender_id] -= tx->amount;
            balances[tx->receiver_id] += tx->amount;
        }
        else{
            return false;
        }
    }
    balances[block.miner_id] += 50;
    balances_temp = balances;    
    return true;
}
