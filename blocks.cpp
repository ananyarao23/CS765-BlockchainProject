#include "simulator.h"

using namespace std;

// calculates the size of each block in Kilobits
double calculateBlockSize(Block *blk)
{
    return (1 + blk->txns.size()) * 8.0;
}


/*
A member function of Peer class
Mine blocks 
            - add valid transactions (for which the sender has sufficient balance) 
            - parent id 

Adds the mined block to a queue which is used to simulate the interarrival delay of blocks, 
and to verify the validity of mined blocks
*/
void Peer::generateBlock()
{
    int num_txns = 0;
    vector<int> txns;
    treeNode *parent_node = blockTree[longestChain];
    for (auto e : memPool)
    {
        Transaction *txn = globalTransactions[e];
        if (parent_node->balances[peerID] >= txn->amount)
        {
            txns.push_back(e);
            num_txns++;
        }
        if (num_txns > 999)
            break;
    }
    Block *blk = new Block(peerID, longestChain, txns);
    globalBlocks[blk->BlkID] = blk;
    total_blocks_generated++;
    int ts;
    do
    {
        ts = generateExponential(simulator->I*1000 / hash_power);
    } while (curr_time + ts < 0);
    
    blockQueue.push({curr_time + ts, blk->BlkID, peerID});
}


/*
Broadcasts received block to its neighbors
*/
void Peer::broadcastBlock(int blkid)
{
    double messagesize = calculateBlockSize(globalBlocks[blkid]);

    for (auto receiver : neighbours)
    {
        int lt = simulator->calculateLatency(peerID, receiver, messagesize);
        sendingQueue.push({curr_time + lt, 1, blkid, receiver});
    }
}


/*
Called on receiving a block
Validates the block, adds it to the tree, removes the seen transactions from the pool if 
the block is a part of the new (or already existing) longest blockchain
If the received block's parent is not found, adds it to the orphan blocks' list to verify later
Checks if the block is the parent of an orphan block
Starts mining on the new longest chain leaf block immediately
Broadcasts the received block to its neighbors
*/
void Peer::receiveBlock(int blkid)
{
    if (globalBlocks.find(blkid) == globalBlocks.end())
    {
        cout << "Block does not exist" << endl;
        return; 
    }
    Block *block = globalBlocks[blkid];
    if (blockSet.find(blkid) == blockSet.end())
    {
        blockSet.insert(blkid);
        int arrivalTime = curr_time;
        timeline[arrivalTime].push_back({blkid, block->parent_id});

        if (blockTree.find(block->parent_id) != blockTree.end())
        {
            map<int, int> balances_temp;
            if (validateBlock(*block, balances_temp))
            {
                total_blocks++;
                // simulator->total_blocks++;
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

                    generateBlock();
                }
                else
                {
                    simulator->forks++;
                }

                broadcastBlock(block->BlkID);
                processOrphanBlocks(*block);
            }
        }
        else
        {
            orphanBlocks.insert(blkid);
        }
    }
}


/*
Given a block, checks is any of the orphan block's are children of this block, if yes
process accordingly
*/
void Peer::processOrphanBlocks(Block &block)
{
    vector<int> toRemove;

    for (int orphan : orphanBlocks)
    {
        if (globalBlocks[orphan]->parent_id == block.BlkID)
        {
            map<int, int> balances_temp;
            if (validateBlock(*globalBlocks[orphan], balances_temp))
            {
                treeNode *parentNode = blockTree[block.BlkID];
                treeNode *orphanChild = new treeNode(parentNode, globalBlocks[orphan]);
                orphanChild->balances = balances_temp;
                leafBlocks.erase(block.BlkID);
                leafBlocks.insert(orphan);
                if (orphanChild->depth > maxDepth)
                {
                    maxDepth = orphanChild->depth;
                    longestChain = orphan;

                    for (int txn : globalBlocks[orphan]->txns)
                    {
                        memPool.erase(txn);
                    }
                }
                broadcastBlock(orphan);
                generateBlock();
                toRemove.push_back(orphan);

                processOrphanBlocks(*globalBlocks[orphan]);
            }
        }
    }
    for (int orphan : toRemove)
    {
        orphanBlocks.erase(orphan);
        total_blocks++;
        simulator->total_blocks++;
    }
}


/*
Validates the blocks -> checks if the sender of every transaction included has sufficient balance
*/
bool Peer::validateBlock(Block &block, map<int, int> &balances_temp)
{
    treeNode *parent = blockTree[block.parent_id];
    map<int, int> balances = parent->balances;
    for (int txn : block.txns)
    {
        Transaction *tx = globalTransactions[txn];
        if (balances[tx->sender_id] >= tx->amount)
        {
            balances[tx->sender_id] -= tx->amount;
            balances[tx->receiver_id] += tx->amount;
        }
        else
        {
            return false;
        }
    }
    balances[block.miner_id] += 50;
    balances_temp = balances;
    return true;
}
