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

