#include "simulator.h"

double calculateBlockSize(Block *blk)
{
    return (1 + blk->txns.size()) * 8.0 / 1000.0;
}

void Peer::generateBlock()
{
    if (memPool.empty()) 
    {
        cout<<"Empty mempool hah"<<endl;
        return;
    }
    int num_txns = 0;
    vector<int> txns;
    for (auto e : memPool)
    {
        txns.push_back(e);
        cout<<"txn pushed in mempool"<<endl;
        num_txns++;
        if (num_txns > 999)
            break;
    }
    Block *blk = new Block(peerID, longestChain, txns);

    globalBlocks[blk->BlkID] = blk;

    int ts = generateExponential(simulator->I/ hash_power);
    blockQueue.push({curr_time + ts, blk->BlkID, peerID});
    cout << "block pushed in block queue" << endl;
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
        int lt = simulator->calculateLatency(peerID, receiver, messagesize);
        sendingQueue.push({curr_time + lt, 1, blkid, receiver});
    }
}

void Peer::receiveBlock(int blkid)
{
    if (globalBlocks.find(blkid) == globalBlocks.end()) {
        return;  // Block ID not found
    }
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
                broadcastBlock(block->BlkID);
                generateBlock();
                processOrphanBlocks(*block);
                
            }
        }
        else
        {
            orphanBlocks.insert(blkid);
        }
    }
}

void Peer::processOrphanBlocks(Block& block) {
    vector<int> toRemove;

    // Process the orphaned blocks
    for (int orphan : orphanBlocks) {
        if (globalBlocks[orphan]->parent_id == block.BlkID) {
            map<int, int> balances_temp;
            if (validateBlock(*globalBlocks[orphan], balances_temp)) {
                treeNode* parentNode = blockTree[block.BlkID];
                treeNode* orphanChild = new treeNode(parentNode, globalBlocks[orphan]);
                orphanChild->balances = balances_temp;
                blockTree[orphan] = orphanChild;
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

                // Recursively process any orphan blocks that are now children of the un-orphaned block
                processOrphanBlocks(*globalBlocks[orphan]);
            }
        }
    }
    for (int orphan : toRemove) {
        orphanBlocks.erase(orphan);
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
