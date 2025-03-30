#include "helper.h"
#include "network.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;

Peer::~Peer()
{
    for (auto it : blockTree)
    {
        delete it.second;
    }
    delete normNet;
    delete genesis_blk;
}

/*
Broadcasts received block's hash to its neighbors
*/

void HonestPeer::broadcastHash(string hash_val)
{
    double messagesize = 0.064;
    for (auto receiver : neighbours)
    {
        cout << "[HPH]: " << peerID << " broadcasting hash: " << hash_val << " to " << receiver << endl;
        int lt = normNet->calculateLatency(peerID, receiver, messagesize);
        normNet->sendingQueue.push({{curr_time + lt, 2, peerID, receiver}, hash_val});
    }
}

void MaliciousPeer::broadcastHash(string hash_val)
{
    double messagesize = 0.064;
    for (auto receiver : neighbours)
    {
        cout << "[MPH]: " << peerID << " broadcasting hash: " << hash_val << " to " << receiver << endl;
        int lt = normNet->calculateLatency(peerID, receiver, messagesize);
        normNet->sendingQueue.push({{curr_time + lt, 2, peerID, receiver}, hash_val});
    }

    for (auto receiver : malicious_neighbours)
    {
        cout << "[MPO]: " << peerID << " broadcasting hash: " << hash_val << " to " << receiver << endl;
        int lt = malNet->calculateLatency(peerID, receiver, messagesize);
        malNet->sendingQueue.push({{curr_time + lt, 2, peerID, receiver}, hash_val});
    }
}

void HonestPeer::receiveHash(string hash_val, int sender_id, int net)
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
        normNet->timeoutQueue.push({{curr_time + Tt, peerID}, hash_val});
        cout << "[HPH]: Peer " << peerID << " received hash: " << hash_val << endl;
        sendGetRequest(hash_val, sender_id);
        return;
    }

    else
    {
        cout << "[HPH]: Peer " << peerID << " received hash again for hash: " << hash_val << endl;
        pending_requests[hash_val].push(sender_id);
    }
}

void MaliciousPeer::receiveHash(string hash_val, int sender_id, int net)
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
        malNet->timeoutQueue.push({{curr_time + Tt, peerID}, hash_val});
        cout << "[MPHprint c/O]: Peer " << peerID << " received hash: " << hash_val << endl;
        sendGetRequest(hash_val, sender_id, net);
        return;
    }

    else
    {
        pending_requests[hash_val].push(sender_id);
    }
}

void HonestPeer::sendGetRequest(string hash_val, int receiver_id, int net)
{
    double messagesize = 0.064;
    cout << "[HPH]: Peer " << peerID << " sending get request to " << receiver_id << " for hash: " << hash_val << endl;
    int lt = normNet->calculateLatency(peerID, receiver_id, messagesize);
    normNet->sendingQueue.push({{curr_time + lt, 3, peerID, receiver_id}, hash_val});
}

void MaliciousPeer::sendGetRequest(string hash_val, int receiver_id, int net)
{
    double messagesize = 0.064;
    if (net == 1)
    {
        cout << "[MPO]: Peer " << peerID << " sending get request to " << receiver_id << " for hash: " << hash_val << endl;

        int lt = malNet->calculateLatency(peerID, receiver_id, messagesize);
        malNet->sendingQueue.push({{curr_time + lt, 3, peerID, receiver_id}, hash_val});
    }
    else
    {
        cout << "[MPH]: Peer " << peerID << " sending get request to " << receiver_id << " for hash: " << hash_val << endl;
        int lt = normNet->calculateLatency(peerID, receiver_id, messagesize);
        normNet->sendingQueue.push({{curr_time + lt, 3, peerID, receiver_id}, hash_val});
    }
}

void HonestPeer::receiveGetRequest(string blk_hash, int rcvr, int net)
{
    cout << "[HPH]: Peer " << peerID << " rcvd get request for hash: " << blk_hash << " from  " << rcvr << endl;
    sendBlock(blk_hash, rcvr, 0);
}

void MaliciousPeer::receiveGetRequest(string blk_hash, int rcvr, int net)
{
    cout << "[HPH]: Peer " << peerID << " rcvd get request for hash: " << blk_hash << " from  " << rcvr << endl;
    if (net == 0)
        return;
    if (malNet->malicious_peers.find(rcvr) != malNet->malicious_peers.end())
    {
        sendBlock(blk_hash, rcvr, net);
    }
}

void HonestPeer::generateBlock()
{
    int num_txns = 0;
    vector<string> txns;
    treeNode *parent_node = blockTree.at(longestChain);
    string coinbase = construct_coinbase(peerID, txnIDctr++);
    txns.push_back(coinbase);

    map<int, int> balances = blockTree.at(longestChain)->balances;

    map<int, int> temp_bals;
    for (auto it : balances)
    {
        temp_bals[it.first] = it.second;
    }

    temp_bals[peerID] += 50; // coinbase

    cout << "SElf check for blk " << blkIDctr << endl;
    cout << "mining on " << longestChain << endl;
    for (auto it : temp_bals)
    {
        cout << it.first << " " << it.second << endl;
    }
    for (string txn : memPool)
    {
        vector<int> txn_vector = parse_txn(txn);
        // txn id, x pays y amt

        if (temp_bals[txn_vector[1]] < txn_vector[3])
        {
            continue;
        }
        temp_bals[txn_vector[1]] -= txn_vector[3];
        temp_bals[txn_vector[2]] += txn_vector[3];

        cout << "AAAAA" << txn << endl;

        // int sender_id = txn_vector[1];
        // int amt = txn_vector[3];

        // if (parent_node->balances[sender_id] >= amt)
        // {
        txns.push_back(txn);
        num_txns++;
        // }
        if (num_txns > 999)
        {
            cout << "Block ID " << blkIDctr << " fully filled" << endl;
            break;
        }
    }

    Block *blk = new Block(blkIDctr++, peerID, longestChain, txns);
    string hash_val = calculateHash(blkIDctr - 1, peerID, longestChain, txns); // calculate hash for this block
    seen_blocks[hash_val] = blk;
    int ts;
    do
    {
        cout << "exponential sampling: " << I * 1000 / hash_power << endl;
        ts = generateExponential(I * 1000 / hash_power);
    } while (curr_time + ts < 0);

    normNet->blockQueue.push({{curr_time + ts, peerID}, hash_val});
    cout << "Block " << blk->BlkID << " generated by HP " << peerID << endl;
    cout << "hash value: " << hash_val << endl;
}

void MaliciousPeer::generateBlock()
{
    if (malNet->ringmasterID != peerID)
        return;
    int num_txns = 0;
    vector<string> txns;
    treeNode *parent_node = blockTree.at(malicious_leaf);
    string coinbase = construct_coinbase(peerID, txnIDctr++);
    txns.push_back(coinbase);

    for (string txn : memPool2)
    {
        vector<int> txn_vector = parse_txn(txn);
        int sender_id = txn_vector[1];
        int amt = txn_vector[3];

        if (parent_node->balances[sender_id] >= amt)
        {
            txns.push_back(txn);
            num_txns++;
        }
        if (num_txns > 999)
        {
            cout << "Block ID " << blkIDctr << " fully filled" << endl;
            break;
        }
    }

    Block *blk = new Block(blkIDctr++, peerID, malicious_leaf, txns);

    string hash_val = calculateHash(blkIDctr - 1, peerID, malicious_leaf, txns); // calculate hash for this block
    seen_blocks[hash_val] = blk;
    int ts;
    do
    {
        cout << "exponential sampling: " << I * 1000 / hash_power << endl;
        ts = generateExponential(I * 1000 / hash_power);
    } while (curr_time + ts < 0);

    malNet->blockQueue.push({{curr_time + ts, peerID}, hash_val});
    cout << "Block " << blk->BlkID << " generated by MP " << peerID << endl;
    // cout << "hash value: " << hash_val << endl;
}

void HonestPeer::sendBlock(string blk_hash, int rcvr, int net)
{
    Block *blk = seen_blocks[blk_hash];
    if (!blk)
    {
        cerr << "Block not seen yet, don't send it stupid" << endl;
        exit(0);
    }
    string message = "%%";
    int a = blk->BlkID;
    blk->BlkID = 5;
    message = message + blk_hash + "%%";
    blk->BlkID = a;
    message = message + to_string(blk->BlkID) + "%%";
    message = message + to_string(blk->miner_id) + "%%";
    message = message + blk->parent_hash + "%%";
    for (auto txn : blk->txns)
    {
        message = message + txn + "%%";
    }
    int ts = normNet->calculateLatency(peerID, rcvr, 1000);
    normNet->sendingQueue.push({{curr_time + ts, 1, peerID, rcvr}, message});
    cout << "Block " << blk->BlkID << " sent to " << rcvr << " by HP " << peerID << "in normNet" << endl;
}

void MaliciousPeer::sendBlock(string blk_hash, int rcvr, int net)
{
    Block *blk = seen_blocks[blk_hash];
    string message = "%%";
    message = message + blk_hash + "%%";
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
        normNet->sendingQueue.push({{curr_time + ts, 1, peerID, rcvr}, message});
        cout << "Block " << blk->BlkID << " sent to " << rcvr << " by MP " << peerID << "in normNet" << endl;
    }
    else
    {
        int ts = malNet->calculateLatency(peerID, rcvr, 1000); // is it correct?
        malNet->sendingQueue.push({{curr_time + ts, 1, peerID, rcvr}, message});
        cout << "Block " << blk->BlkID << " sent to " << rcvr << " by MP " << peerID << "in malNet" << endl;
    }
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

/*
Called on receiving a block
Validates the block, adds it to the tree, removes the seen transactions from the pool if
the block is a part of the new (or already existing) longest blockchain
If the received block's parent is not found, adds it to the orphan blocks' list to verify later
Checks if the block is the parent of an orphan block
Starts mining on the new longest chain leaf block immediately
Broadcasts the received block to its neighbors
*/

void HonestPeer::receiveBlock(int sender_id, string block)
{
    int idx = 2;
    while (block[idx] != '%')
        idx++;
    string hash_val = block.substr(2, idx - 2);
    if (seen_blocks.find(hash_val) != seen_blocks.end())
    {
        return; // block already received
    }

    if (pending_requests.find(hash_val) != pending_requests.end())
        pending_requests.erase(hash_val);
    if (timeouts.find(hash_val) != timeouts.end())
        timeouts.erase(hash_val);
    idx = idx + 2;
    int idx1 = idx;
    while (block[idx] != '%')
        idx++;
    int blk_id = stoi(block.substr(idx1, idx - idx1));
    idx = idx + 2;
    idx1 = idx;
    while (block[idx] != '%')
        idx++;
    int miner_id = stoi(block.substr(idx1, idx - idx1));
    idx = idx + 2;
    idx1 = idx;
    while (block[idx] != '%')
        idx++;
    string parent_hash = block.substr(idx1, idx - idx1);
    idx = idx + 2;
    idx1 = idx;
    vector<string> transactions;
    cout << block << endl;
    while (idx < block.length())
    {
        while (block[idx] != '%')
            idx++;
        string txn = block.substr(idx1, idx - idx1);
        idx = idx + 2;
        idx1 = idx;
        transactions.push_back(txn);
        cout << "LLLLLLLL" << txn << endl;
    }
    Block *block_ptr = new Block(blk_id, miner_id, parent_hash, transactions);

    cout << "Block " << block_ptr->BlkID << " received from " << sender_id << " by HP " << peerID << endl;
    seen_blocks[hash_val] = block_ptr;

    if (blockTree.find(parent_hash) != blockTree.end())
    {
        int arrivalTime = curr_time;
        int parent_id = seen_blocks[parent_hash]->BlkID;
        timeline[arrivalTime].push_back({blk_id, parent_id});
        map<int, int> balances_temp;
        if (validateBlock(block_ptr, balances_temp))
        {
            valid_timeline[arrivalTime].push_back({blk_id, parent_id});
            treeNode *parentNode = blockTree.at(parent_hash);
            treeNode *child = new treeNode(parentNode, blk_id, peerID);
            child->hash = hash_val;
            cout << child << " " << hash_val << " tree node" << peerID << endl;
            child->balances = balances_temp;
            blockTree[hash_val] = child;
            leafBlocks.erase(parent_hash);
            leafBlocks.insert(hash_val);

            if (child->depth > maxDepth)
            {

                maxDepth = child->depth;
                longestChain = hash_val;

                for (string txn : transactions)
                {
                    memPool.erase(txn);
                }
                broadcastHash(hash_val);
                generateBlock();
            }
            else
            {
                // simulator->forks++;
            }

            processOrphanBlocks(hash_val);
        }
    }
    else
    {
        orphanBlocks.insert(hash_val);
    }
}

void MaliciousPeer::receiveBlock(int sender_id, string block)
{
    int idx = 2;
    while (block[idx] != '%')
        idx++;
    string hash_val = block.substr(2, idx - 2);
    if (seen_blocks.find(hash_val) != seen_blocks.end())
    {
        return; // block already received
    }
    if (pending_requests.find(hash_val) != pending_requests.end())
        pending_requests.erase(hash_val);
    if (timeouts.find(hash_val) != timeouts.end())
        timeouts.erase(hash_val);
    idx = idx + 2;
    int idx1 = idx;
    while (block[idx] != '%')
        idx++;
    int blk_id = stoi(block.substr(idx1, idx - idx1));
    idx = idx + 2;
    idx1 = idx;
    while (block[idx] != '%')
        idx++;
    int miner_id = stoi(block.substr(idx1, idx - idx1));
    idx = idx + 2;
    idx1 = idx;
    while (block[idx] != '%')
        idx++;
    string parent_hash = block.substr(idx1, idx - idx1);
    idx = idx + 2;
    idx1 = idx;
    vector<string> transactions;
    while (idx < block.length())
    {
        while (block[idx] != '%')
            idx++;
        string txn = block.substr(idx1, idx - idx1);
        idx = idx + 2;
        idx1 = idx;
        transactions.push_back(txn);
    }

    Block *block_ptr = new Block(blk_id, miner_id, parent_hash, transactions);
    cout << "Block " << block_ptr->BlkID << " received from " << sender_id << " by MP " << peerID << endl;
    seen_blocks[hash_val] = block_ptr;

    if (blockTree.find(parent_hash) != blockTree.end())
    {
        int arrivalTime = curr_time;
        int parent_id = seen_blocks[parent_hash]->BlkID;
        timeline[arrivalTime].push_back({blk_id, parent_id});
        map<int, int> balances_temp;
        if (validateBlock(block_ptr, balances_temp))
        {
            cout << "Block added to tree" << endl;
            valid_timeline[arrivalTime].push_back({blk_id, parent_id});
            treeNode *parentNode = blockTree.at(parent_hash);
            treeNode *child = new treeNode(parentNode, blk_id, peerID);
            child->hash = hash_val;
            cout << child << " " << hash_val << " tree node" << peerID << endl;
            child->balances = balances_temp;
            blockTree[hash_val] = child;
            if (malNet->malicious_peers.find(block_ptr->miner_id) == malNet->malicious_peers.end())
            {
                leafBlocks.erase(parent_hash);
                leafBlocks.insert(hash_val);
                if (child->depth > maxDepth)
                {

                    maxDepth = child->depth;
                    longestChain = hash_val;

                    for (string txn : transactions)
                    {
                        memPool.erase(txn);
                    }
                    broadcastHash(hash_val);
                    generateBlock();
                }
                else
                {
                    // simulator->forks++;
                }
                processOrphanBlocks(hash_val);
            }
            else
            {
                malicious_leaf = hash_val;
                malicious_len++;
            }
        }
    }
    else
    {
        orphanBlocks.insert(hash_val);
    }
}

/*
Given a block, checks is any of the orphan block's are children of this block, if yes
process accordingly
*/
void Peer::processOrphanBlocks(string hash_val)
{
    Block *block = seen_blocks[hash_val];
    vector<string> toRemove;

    for (string orphan : orphanBlocks)
    {
        if (seen_blocks[orphan]->parent_hash == hash_val)
        {
            map<int, int> balances_temp;
            if (validateBlock(seen_blocks[orphan], balances_temp))
            {
                valid_timeline[curr_time].push_back({seen_blocks[orphan]->BlkID, block->BlkID});
                treeNode *parentNode = blockTree.at(hash_val);
                treeNode *orphanChild = new treeNode(parentNode, seen_blocks[orphan]->BlkID, peerID);
                cout << "called orphan blocks yay" << endl;
                cout << orphanChild << " " << orphan << " tree node" << peerID << endl;
                orphanChild->hash = orphan;
                orphanChild->balances = balances_temp;
                blockTree[orphan] = orphanChild;
                leafBlocks.erase(hash_val);
                leafBlocks.insert(orphan);
                if (orphanChild->depth > maxDepth)
                {
                    maxDepth = orphanChild->depth;
                    longestChain = orphan;

                    for (string txn : seen_blocks[orphan]->txns)
                    {
                        memPool.erase(txn);
                    }
                }
                broadcastHash(orphan);
                generateBlock();
                toRemove.push_back(orphan);

                processOrphanBlocks(orphan);
            }
        }
    }
    for (string orphan : toRemove)
    {
        orphanBlocks.erase(orphan);
    }
    cout << "Orphan blocks processed for" << hash_val << endl;
}

/*
Validates the blocks -> checks if the sender of every transaction included has sufficient balance
*/
bool Peer::validateBlock(Block *block, map<int, int> &balances_temp)
{
    cout << "validating block " << block->BlkID << endl;
    treeNode *parent = blockTree.at(block->parent_hash);
    map<int, int> balances = parent->balances;

    cout << "no of txns" << block->txns.size() << endl;
    int ctr = 0;

    cout << "balances before block validation" << endl;
    for (auto it : balances)
    {
        cout << it.first << " " << it.second << endl;
    }

    for (auto txn : block->txns)
    {
        vector<int> txn_vec = parse_txn(txn);
        // txn id, x pays y amt
        if (txn_vec[1] == -1)
        {
            balances[txn_vec[2]] += 50;
            cout << "coinbase" << endl;
            continue;
        }
        cout << "transaction" << txn << endl;
        if (balances[txn_vec[1]] >= txn_vec[3])
        {
            cout << "txn " << ctr << " OK" << endl;
            balances[txn_vec[1]] -= txn_vec[3];
            balances[txn_vec[2]] += txn_vec[3];
        }
        else
        {
            cout << "txn " << ctr << " BADBAD" << endl;
            return false;
        }
        ctr++;
    }
    balances[block->miner_id] += 50;
    balances_temp = balances;
    cout << "Block " << block->BlkID << " validated" << endl;
    return true;
}

/* For visualization and result compilation */
void Peer::writeBlockTimesToFile()
{
    filesystem::create_directories("output/tree"); // Ensure directory exists
    filesystem::create_directories("output/valid_tree");

    // Writing to "output/tree/block_times_<peerID>.txt"
    {
        ofstream outFile("output/tree/block_times_" + to_string(peerID) + ".txt");
        if (!outFile.is_open())
        {
            cerr << "Error: Could not open output/tree file for writing\n";
            return;
        }

        outFile << "BlockID : Time: Parent BlockID " << endl;
        outFile << "---------------------------------" << endl;
        outFile << "0 : 0 : -1" << endl;

        for (const auto &entry : timeline)
        {
            for (const auto &id : entry.second)
            {
                outFile << id.first << " : " << entry.first << " : " << id.second << "\n";
            }
        }
        outFile.close();
    }

    // Writing to "output/valid_tree/block_times_<peerID>.txt"
    {
        ofstream outFile("output/valid_tree/block_times_" + to_string(peerID) + ".txt");
        if (!outFile.is_open())
        {
            cerr << "Error: Could not open output/valid_tree file for writing\n";
            return;
        }

        outFile << "BlockID : Time: Parent BlockID " << endl;
        outFile << "---------------------------------" << endl;
        outFile << "0 : 0 : -1" << endl;

        for (const auto &entry : valid_timeline)
        {
            for (const auto &id : entry.second)
            {
                outFile << id.first << " : " << entry.first << " : " << id.second << "\n";
            }
        }
        outFile.close();
    }
}

/* Takes as argument the global genesis block pointer and initializes the tree */
void HonestPeer::createTree(Block *genesis_block)
{
    genesis_blk = new treeNode(nullptr, genesis_block->BlkID, peerID);
    string hash_val = calculateHash(0, -1, "parent_of_genesis", {});
    cout << genesis_blk << " " << hash_val << " tree node" << peerID << endl;
    genesis_blk->hash = hash_val;
    seen_blocks[hash_val] = genesis_block;
    blockTree[hash_val] = genesis_blk;
    longestChain = hash_val;
    leafBlocks.insert(hash_val);
}

void MaliciousPeer::createTree(Block *genesis_block)
{
    genesis_blk = new treeNode(nullptr, genesis_block->BlkID, peerID);
    string hash_val = calculateHash(0, -1, "parent_of_genesis", {});
    genesis_blk->hash = hash_val;
    cout << genesis_blk << " " << hash_val << " tree node" << peerID << endl;
    seen_blocks[hash_val] = genesis_block;
    blockTree[hash_val] = genesis_blk;
    longestChain = hash_val;
    malicious_leaf = hash_val;
    leafBlocks.insert(hash_val);
}

/* Helper function to determine whether or not to forward a message */
bool Peer::query(int txn_id)
{
    if (transactionSet.find(txn_id) != transactionSet.end())
        return false;
    transactionSet.insert(txn_id);
    return true;
}

// void Peer::treeAnalysis()
// {
//     map<int, bool> inBlockChain;
//     string block_hash = longestChain;
//     while (blkid != 0)
//     {
//         inBlockChain[blkid] = true;
//         blkid = globalBlocks[blkid]->parent_id;
//     }
//     inBlockChain[0] = true;

//     vector<int> branch_len;
//     for (auto leaf : leafBlocks)
//     {
//         int len = 0;
//         while (!inBlockChain[leaf])
//         {
//             len++;
//             leaf = globalBlocks[leaf]->parent_id;
//         }
//         branch_len.push_back(len);
//     }
// }

int Peer::blocks_in_longest_chain()
{
    int num_blocks = 0;

    // Traverse backwards to find which nodes contributed blocks to the longest chain
    treeNode *currNode = blockTree.at(longestChain);
    while (currNode->block_id != 0)
    {
        string hash = currNode->hash;
        if (seen_blocks[hash]->miner_id == peerID)
        {
            num_blocks++;
        }

        currNode = blockTree.at(currNode->parent_hash);
    }

    return num_blocks;
}

void HonestPeer::addBlocktoTree(string hash_val)
{
    Block *block = seen_blocks[hash_val];
    treeNode *parentNode = blockTree.at(block->parent_hash);
    cout << "Parent node: " << parentNode->block_id << endl;
    cout << "Parent hash: " << parentNode->hash << endl;
    treeNode *child = new treeNode(parentNode, block->BlkID, peerID);
    child->hash = hash_val;
    cout << child << " " << hash_val << " tree node" << peerID << endl;
    cout << "Child node: " << child->block_id << endl;
    cout << "Child hash: " << child->hash << endl;

    for (string txn : block->txns)
    {
        vector<int> txn_vec = parse_txn(txn);
        if (txn_vec[1] == -1)
        {
            child->balances[txn_vec[2]] += txn_vec[3];
            continue;
        }
        child->balances[txn_vec[1]] -= txn_vec[3];
        child->balances[txn_vec[2]] += txn_vec[3];
    }

    child->balances[peerID] += 50;
    blockTree[hash_val] = child;
    maxDepth = child->depth;
    longestChain = hash_val;
    leafBlocks.erase(block->parent_hash);
    leafBlocks.insert(hash_val);

    for (string txn : block->txns)
    {
        memPool.erase(txn);
    }
}

void MaliciousPeer::addBlocktoTree(string hash_val)
{
    Block *block = seen_blocks[hash_val];
    treeNode *parentNode = blockTree.at(block->parent_hash);
    treeNode *child = new treeNode(parentNode, block->BlkID, peerID);
    child->hash = hash_val;
    cout << child << " " << hash_val << " tree node" << peerID << endl;
    for (string txn : block->txns)
    {
        vector<int> txn_vec = parse_txn(txn);
        if (txn_vec[1] == -1)
        {
            child->balances[txn_vec[2]] += txn_vec[3];
            continue;
        }
        child->balances[txn_vec[1]] -= txn_vec[3];
        child->balances[txn_vec[2]] += txn_vec[3];
    }

    child->balances[peerID] += 50;
    blockTree[hash_val] = child;
    malicious_len++;
    malicious_leaf = hash_val;
    for (string txn : block->txns)
    {
        memPool2.erase(txn);
    }
}
