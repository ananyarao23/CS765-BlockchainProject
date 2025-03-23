#include "structure.h"

using namespace std;

void MaliciousPeer::receiveGetRequest(string blk_hash, int rcvr, int net) const override
{
    if (malNet->malicious_peers.find(rcvr) != malNet->malicious_peers.end())
    {
        sendBlock(blk_hash, rcvr, net);
    }
}

void HonestPeer::receiveGetRequest(string blk_hash, int rcvr) override
{
    sendBlock(blk_hash, rcvr, 0);
}

void MaliciousPeer::sendBlock(string blk_hash, int rcvr, int net) override
{
    Block *blk = seen_blocks[blk_hash];
    string message = "%%";
    message = message + to_string(blk->BlkID) + "%%";
    message = message + to_string(blk->miner_id) + "%%";
    message = message + blk->parent_hash + "%%";
    for (auto txn : blk->txns)
    {
        message = message + txn + "%%";
    }
    if (net == 0)
    {
        int ts = normNet->calculateLatency(peerID, rcvr, 1000);
        normNet->sendingQueue({{curr_time + ts, 1, rcvr}, message});
    }
    else
    {
        int ts = malNet->calculateLatency(peerID, rcvr, 1000); // is it correct?
        malNet->sendingQueue({{curr_time + ts, 1, rcvr}, message});
    }
}

void HonestPeer::sendBlock(string blk_hash, int rcvr)
{
    Block *blk = seen_blocks[blk_hash];
    string message = "%%";
    message = message + to_string(blk->BlkID) + "%%";
    message = message + to_string(blk->miner_id) + "%%";
    message = message + blk->parent_hash + "%%";
    int blk_size = calculateBlockSize(blk);
    int ts = normNet->calculateLatency(peerID, rcvr, blk_size);
    normNet->sendingQueue({{curr_time + ts, 1, rcvr}, message});
}

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
    vector<string> txns;
    treeNode *parent_node = blockTree[longestChain];
    string coinbase = construct_coinbase(peerID, txnIDctr++);
    txns.push_back(coinbase);

    // havent fixed yet 

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
    int ts;
    do
    {
        ts = generateExponential(simulator->I * 1000 / hash_power);
    } while (curr_time + ts < 0);

    blockQueue.push({curr_time + ts, blk->BlkID, peerID});
}

/*
Broadcasts received block's hash to its neighbors
*/
void Peer::broadcastHash(string hash_val)
{
    double messagesize = 0.064;

    for (auto receiver : neighbours)
    {
        int lt = simulator->calculateLatency(peerID, receiver, messagesize);
        sendingQueue.push({{curr_time + lt, 1, receiver}, hash_val});
    }
}

void Peer::receiveHash(string hash_val, int sender_id)
{

    // If the full block has already been received, discard the hash
    if (seen_blocks.find(hash_val) != seen_blocks.end())
    {
        return;
    }

    // If this is the first time seeing the hash, send get request to retrieve full block
    else if (pending_requests[hash_val].empty())
    {
        pending_requests[hash_val].push(sender_id); // entry has to be deleted when tiemout is over
        timeouts[hash_val] = curr_time + Tt;        // entry has to be deleted when timeout is over
        timeoutQueue.push({{curr_time + Tt, peerID}, hash_val});
        sendGetRequest(hash_val, sender_id);
        return;
    }

    else
    {
        pending_requests[hash_val].push(sender_id);
    }
}

void Peer::sendGetRequest(string hash_val, int receiver_id)
{
    double messagesize = 0.064;
    int lt = simulator->calculateLatency(peerID, receiver_id, messagesize);
    sendingQueue.push({{curr_time + lt, 2, receiver}, hash_val});
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

void Peer::receiveBlock(string hash_val, int sender_id, string block)
{
    if (seen_blocks.find(hash_val) != seen_blocks.end())
    {
        return; // block already received
    }

    seen_blocks[hash_val] = block;
    pending_requests.erase(hash_val);
    timeouts.erase(hash_val);
    int idx = 2;
    while(block[idx] != '%') idx++;
    int blk_id = stoi(block.substr(2,idx-2));
    int idx1 = idx + 2;
    while(block[idx] != '%') idx++;
    int miner_id = stoi(block.substr(idx1,idx-idx1));
    idx1 = idx + 2;
    while(block[idx] != '%') idx++;
    string parent_hash = block.substr(idx1,idx-idx1);
    idx1 = idx + 2;
    vector<string> transactions;
    while(idx != block.length()-2)
    {
        while(block[idx] != '%') idx++;
        string txn = block.substr(idx1,idx-idx1);
        transactions.push_back(txn);
    }

    int arrivalTime = curr_time;
    timeline[arrivalTime].push_back({blk_id, parent_hash});

    if (blockTree.find(parent_hash) != blockTree.end())
    {
        map<int, int> balances_temp;
        if (validateBlock(transactions, balances_temp))
        {
            valid_timeline[arrivalTime].push_back({blk_id, parent_hash});
            treeNode *parentNode = blockTree[parent_hash];
            treeNode *child = new treeNode(parentNode, block);
            child->balances = balances_temp;
            blockTree[hash_val] = child;
            leafBlocks.erase(parent_hash);
            leafBlocks.insert(blk_id);

            if (child->depth > maxDepth)
            {

                maxDepth = child->depth;
                longestChain = blk_id;

                for (string txn : transactions)
                {
                    memPool.erase(txn);
                }

                generateBlock();
            }
            else
            {
                simulator->forks++;
            }

            broadcastBlock(hash_val);
            processOrphanBlocks(hash_val);
        }
    }
    else
    {
        orphanBlocks.insert(blk_id);
    }
    

}

// void Peer::receiveBlock(string hash_val, int sender_id, string block)
// {
//     if (seen_blocks.find(hash_val) != seen_blocks.end())
//     {
//         return; // block already received
//     }
//     if (pending_requests[hash_val].front() != sender_id)
//     {
//         cout << "Block not requested" << endl; // block not requested
//         return;
//     }
//     seen_blocks[hash_val] = block;
//     pending_requests.erase(hash_val);
//     timeouts.erase(hash_val);

//     int arrivalTime = curr_time;
//     timeline[arrivalTime].push_back({block->BlkID, block->parent_hash});

//     if (blockTree.find(block->parent_hash) != blockTree.end())
//     {
//         map<int, int> balances_temp;
//         if (validateBlock(*block, balances_temp))
//         {
//             valid_timeline[arrivalTime].push_back({block->BlkID, block->parent_hash});
//             treeNode *parentNode = blockTree[block->parent_hash];
//             treeNode *child = new treeNode(parentNode, block);
//             child->balances = balances_temp;
//             blockTree[block->calculateHash(block->BlkID)] = child;
//             leafBlocks.erase(block->parent_id);
//             leafBlocks.insert(block->BlkID);

//             if (child->depth > maxDepth)
//             {

//                 maxDepth = child->depth;
//                 longestChain = blkid;

//                 for (int txn : block->txns)
//                 {
//                     memPool.erase(txn);
//                 }

//                 generateBlock();
//             }
//             else
//             {
//                 simulator->forks++;
//             }

//             broadcastBlock(block->BlkID);
//             processOrphanBlocks(*block);
//         }
//     }
//     else
//     {
//         orphanBlocks.insert(blkid);
//     }
// }

/*
Given a block, checks is any of the orphan block's are children of this block, if yes
process accordingly
*/
void Peer::processOrphanBlocks(string hash_val, vector<string> transactions)
{
    string block = seen_blocks[hash_val];
    vector<int> toRemove;

    for (string orphan : orphanBlocks)
    {
        if (seen_blocks[orphan] == block.BlkID)
        {
            map<int, int> balances_temp;
            if (validateBlock(*globalBlocks[orphan], balances_temp))
            {
                valid_timeline[curr_time].push_back({orphan, block.BlkID});
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
    }
}

/*
Validates the blocks -> checks if the sender of every transaction included has sufficient balance
*/
bool Peer::validateBlock(vector<string> transactions, map<int, int> &balances_temp)
{
    treeNode *parent = blockTree[block.parent_id];
    map<int, int> balances = parent->balances;
    
    for (auto txn : transactions)
    {
        vector<int> txn_vec = parse_txn(txn);
        
    }
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
