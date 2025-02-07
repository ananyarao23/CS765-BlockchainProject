#include "simulator.h"
#include <random>
#include <ctime>
#include <cstdlib>
#include <iostream>
using namespace std;

void P2P::receiveBlock(){
    while(!sendingQueue.empty()){
        Block block = sendingQueue.front();
        sendingQueue.pop();

        //check if receiving peer has already seen the block
        if(blockSet.find(block.BlkID) == blockSet.end()){
            blockSet.insert(block.BlkID);

            if (leafBlocks.find(block.parent_id) != leafBlocks.end()){

                //now validate transactions in the block
                if(validateBlock(block)){

                    treeNode* parentNode = blockTree[block.parent_id];
                    treeNode* child = new treeNode(parentNode, &block);
                    child->balances = block.balances;
                    blockTree[block.BlkID] = child;

                    leafBlocks.erase(block.parent_id);
                    leafBlocks.insert(block.BlkID);

                    if (child->depth > maxDepth){
                        maxDepth = child->depth;
                        //updte longest chain

                        for (Transaction txn : block.txns){
                            memPool.erase(txn.txnID);
                        }
                    }

                    processOrphanBlocks(block);
                    broadcastBlock(block.BlkID);
                }
            }
            else {
                orphanBlocks.insert(block.BlkID);
            }
        }
    }
}

void P2P::processOrphanBlocks(Block& block) {
    // Flag to track if any orphan blocks were processed
    bool processedAnyOrphan = false;

    // Process the orphaned blocks
    for (int orphan : orphanBlocks) {
        if (blockTree[orphan]->parent_id == block.BlkID) {
            processedAnyOrphan = true;
            if (validateBlock(blockTree[orphan])) {
                treeNode* parentNode = blockTree[block.BlkID];
                treeNode* orphanChild = new treeNode(parentNode, &blockTree[orphan]);
                orphanChild->balances = blockTree[orphan]->balances;
                blockTree[orphan] = orphanChild;
                leafBlocks.erase(blockTree[orphan]->parent_id);
                leafBlocks.insert(orphan);
                orphanBlocks.erase(orphan);

                // Recursively process any orphan blocks that are now children of the un-orphaned block
                processOrphanBlocks(blockTree[orphan]);
            }
        }
    }

    if (!processedAnyOrphan) {
        return;
    }
}


bool P2P::validateBlock(Block& block){
    //to check go the chain in the tree which has the parent then at parent we have balances of each peer stored, validate each transaction
    treeNode* parent = blockTree[block.parent_id];
    map <int, int> balances = parent->balances;
    for(Transaction tx : block.txns){
        if(balances[tx.sender_id] >= tx.amount){
            balances[tx.sender_id] -= tx.amount;
            balances[tx.receiver_id] += tx.amount;
        }
        else{
            return false;
        }
    }
    block.balances = balances;
    return true;
}