#include "helper.h"
#include "network.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;

/*
parses the string, corresponding to a block, sent over network
all the attributes of block stored in such string are separated using delimiter "%%"
this string begins and end with "%%"
*/

pair<string, Block *> parse_block(const string &block)
{
    int idx = 2;
    while (block[idx] != '%')
        idx++;
    string hash_val = block.substr(2, idx - 2); // extract the hash of the block

    idx = idx + 2;
    int idx1 = idx;
    while (block[idx] != '%')
        idx++;
    int blk_id = stoi(block.substr(idx1, idx - idx1)); // extract the block_id of the block
    idx = idx + 2;
    idx1 = idx;
    while (block[idx] != '%')
        idx++;
    int miner_id = stoi(block.substr(idx1, idx - idx1)); // extract the miner_id of the block
    idx = idx + 2;
    idx1 = idx;
    while (block[idx] != '%')
        idx++;
    string parent_hash = block.substr(idx1, idx - idx1); // extract the parent_hash of the bloc
    idx = idx + 2;
    idx1 = idx;
    vector<string> transactions;
    while (idx < block.length()) // extract the transactions
    {
        while (block[idx] != '%')
            idx++;
        string txn = block.substr(idx1, idx - idx1);
        idx = idx + 2;
        idx1 = idx;
        transactions.push_back(txn);
    }
    Block *block_ptr = new Block(blk_id, miner_id, parent_hash, transactions); // make a block object
    return {hash_val, block_ptr};
}

/*
Honest Peer - Broadcasts received block's hash to its neighbors (in original network)
*/
void HonestPeer::broadcastHash(string hash_val, int net)
{
    double messagesize = 0.064;
    for (auto receiver : neighbours)
    {
        int lt = normNet->calculateLatency(peerID, receiver, messagesize);
        normNet->sendingQueue.push({{curr_time + lt, 2, peerID, receiver}, hash_val});
    }
}


/*
Malicious Peer - Broadcats recieved block's hash to it's neighbors in both networks (overlay and original)
*/
void MaliciousPeer::broadcastHash(string hash_val, int net)
{
    double messagesize = 0.064;
    if (net == 0)
    {
        for (auto receiver : neighbours)
        {
            int lt = normNet->calculateLatency(peerID, receiver, messagesize);
            normNet->sendingQueue.push({{curr_time + lt, 2, peerID, receiver}, hash_val});
        }
    }

    for (auto receiver : malicious_neighbours)
    {
        int lt = malNet->calculateLatency(peerID, receiver, messagesize);
        malNet->sendingQueue.push({{curr_time + lt, 2, peerID, receiver}, hash_val});
    }
}


/*
Honest Peer - on receiveing a hash, the received send get request into the network, intended for the sender of hash
*/
void HonestPeer::receiveHash(string hash_val, int sender_id, int)
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
        sendGetRequest(hash_val, sender_id);
        return;
    }

    else
    {
        pending_requests[hash_val].push(sender_id);
    }
}


/*
Malicious Peer - on receiveing a hash, the received send get request into the network, intended for the sender of hash
*/
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
        sendGetRequest(hash_val, sender_id, net);
        return;
    }

    else
    {
        pending_requests[hash_val].push(sender_id);
    }
}


/*
Honest Peer - Send get request in the network net, also simulates a latentcy
*/
void HonestPeer::sendGetRequest(string hash_val, int receiver_id, int net)
{
    double messagesize = 0.064;
    int lt = normNet->calculateLatency(peerID, receiver_id, messagesize);
    normNet->sendingQueue.push({{curr_time + lt, 3, peerID, receiver_id}, hash_val});
}

/*
Malicious Peer - Send get request in the network net, also simulates a latentcy
*/
void MaliciousPeer::sendGetRequest(string hash_val, int receiver_id, int net)
{
    double messagesize = 0.064;
    if (net == 1)
    {

        int lt = malNet->calculateLatency(peerID, receiver_id, messagesize);
        malNet->sendingQueue.push({{curr_time + lt, 3, peerID, receiver_id}, hash_val});
    }
    else
    {
        int lt = normNet->calculateLatency(peerID, receiver_id, messagesize);
        normNet->sendingQueue.push({{curr_time + lt, 3, peerID, receiver_id}, hash_val});
    }
}

/*
Honest Peer - Send block to the sender of get request over the network net
*/
void HonestPeer::receiveGetRequest(string blk_hash, int rcvr, int net)
{
    sendBlock(blk_hash, rcvr, 0);
}

/*
Malicious Peer - Send block to the sender of get request over the network net
                 it serves to the get request of an honest neighbour if and only if eclise attack is not enabled
*/
void MaliciousPeer::receiveGetRequest(string blk_hash, int rcvr, int net)
{
    if (!eclipse_attack)
    {
        sendBlock(blk_hash, rcvr, net);
        return;
    }
    if (malNet->malicious_peers.find(rcvr) != malNet->malicious_peers.end())
    {
        sendBlock(blk_hash, rcvr, net);
    }
    else if (blocks_to_release.find(blk_hash) != blocks_to_release.end())
    {
        sendBlock(blk_hash, rcvr, net);
    }
}

/*
Honest Peer - Mine block on longest chain's leaf and add it to network's block queue to simulate interarrival
            time, based on the hash power of the peer
*/
void HonestPeer::generateBlock()
{
    if (normNet->stop)
        return;
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

    for (string txn : memPool)
    {
        vector<int> txn_vector = parse_txn(txn);

        int sender_id = txn_vector[1];
        int receiver_id = txn_vector[2];
        int amt = txn_vector[3];

        if (temp_bals[sender_id] < amt)
        {
            continue;
        }
        temp_bals[sender_id] -= amt;
        temp_bals[receiver_id] += amt;

        txns.push_back(txn);
        num_txns++;

        if (num_txns > 999)
        {
            break;
        }
    }

    Block *blk = new Block(blkIDctr++, peerID, longestChain, txns);
    string hash_val = calculateHash(blkIDctr - 1, peerID, longestChain, txns); // calculate hash for this block
    seen_blocks[hash_val] = blk;
    int ts;
    do
    {
        ts = generateExponential(I * 1000 / hash_power);
    } while (curr_time + ts < 0);

    normNet->blockQueue.push({{curr_time + ts, peerID}, hash_val});
}


/*
Malicious Peer - Mines block only if the function is called by the ringmaster of overlay network
                and add it to blockQueue to simulate interarrival time
NOTE - ringmaster will utilise the hash power of all malicious nodes
*/
void MaliciousPeer::generateBlock()
{
    if (malNet->stop)
        return;
    if (malNet->ringmasterID != peerID)
        return;
    int num_txns = 0;
    vector<string> txns;
    treeNode *parent_node = blockTree.at(malNet->malicious_leaf);
    string coinbase = construct_coinbase(peerID, txnIDctr++);
    txns.push_back(coinbase);

    map<int, int> balances = parent_node->balances;
    map<int, int> temp_bals;
    for (auto it : balances)
    {
        temp_bals[it.first] = it.second;
    }
    temp_bals[peerID] += 50; // coinbase
    for (string txn : memPool2)
    {
        vector<int> txn_vector = parse_txn(txn);
        int sender_id = txn_vector[1];
        int receiver_id = txn_vector[2];
        int amt = txn_vector[3];

        if (temp_bals[sender_id] < amt)
        {
            continue;
        }
        temp_bals[sender_id] -= amt;
        temp_bals[receiver_id] += amt;

        txns.push_back(txn);
        num_txns++;

        if (num_txns > 999)
        {
            break;
        }
    }

    Block *blk = new Block(blkIDctr++, peerID, malNet->malicious_leaf, txns);
    string hash_val = calculateHash(blkIDctr - 1, peerID, malNet->malicious_leaf, txns); // calculate hash for this block
    seen_blocks[hash_val] = blk;
    int ts;
    do
    {
        ts = generateExponential(I * 1000 / hash_power);

    } while (curr_time + ts < 0);

    malNet->blockQueue.push({{curr_time + ts, peerID}, hash_val});
}


/*
Honest Peer - Pack the block into a string, append all attributes and transactions, in string format, 
            with a delimiter ("%%") and it to the network's sending queue with simulated latency
*/
void HonestPeer::sendBlock(string blk_hash, int rcvr, int net)
{
    Block *blk = seen_blocks[blk_hash];
    if (!blk)
    {
        exit(0);
    }
    string message = "%%";
    message = message + blk_hash + "%%";
    message = message + to_string(blk->BlkID) + "%%";
    message = message + to_string(blk->miner_id) + "%%";
    message = message + blk->parent_hash + "%%";
    for (auto txn : blk->txns)
    {
        message = message + txn + "%%";
    }
    int ts = normNet->calculateLatency(peerID, rcvr, 1000);
    normNet->sendingQueue.push({{curr_time + ts, 1, peerID, rcvr}, message});
}

/*
Malicious Peer - Pack the block into a string, append all attributes and transactions, in string format, 
            with a delimiter ("%%") and it to the network's sending queue with simulated latency
*/

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
    }
    else
    {
        int ts = malNet->calculateLatency(peerID, rcvr, 1000); // is it correct?
        malNet->sendingQueue.push({{curr_time + ts, 1, peerID, rcvr}, message});
    }
}

// calculates the size of each block in Kilobits
double calculateBlockSize(Block *blk)
{
    return (1 + blk->txns.size()) * 8.0;
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

void HonestPeer::receiveBlock(int sender_id, string block)
{
    pair<string, Block *> block_pair = parse_block(block);
    string hash_val = block_pair.first;
    Block *block_ptr = block_pair.second;

    if (seen_blocks.find(hash_val) != seen_blocks.end())
    {
        return; // block already received
    }

    if (pending_requests.find(hash_val) != pending_requests.end())
        pending_requests.erase(hash_val);
    if (timeouts.find(hash_val) != timeouts.end())
        timeouts.erase(hash_val);

    int blk_id = block_ptr->BlkID;
    string parent_hash = block_ptr->parent_hash;
    vector<string> transactions = block_ptr->txns;
    seen_blocks[hash_val] = block_ptr;

    if (blockTree.find(parent_hash) != blockTree.end())
    {
        int arrivalTime = curr_time;
        int parent_id = seen_blocks[parent_hash]->BlkID;
        timeline[arrivalTime].push_back({blk_id, parent_id});
        map<int, int> balances_temp;
        if (validateBlock(block_ptr, balances_temp))
        {
            valid_timeline[arrivalTime].push_back({blk_id, parent_id, 0});
            treeNode *parentNode = blockTree.at(parent_hash);
            treeNode *child = new treeNode(parentNode, blk_id, peerID);
            child->hash = hash_val;
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
                broadcastHash(hash_val, 0);
                generateBlock();
            }

            processOrphanBlocks(hash_val);
        }
    }
    else
    {
        orphanBlocks.insert(hash_val);
    }
}

/*
Malicious Peer - Called on receiving a block
                Validates the block, adds it to the tree, removes the seen transactions from the pool if
                the block is a part of the new (or already existing) longest blockchain
                If the received block's parent is not found, adds it to the orphan blocks' list to verify later
                Checks if the block is the parent of an orphan block
                Starts mining on the new longest chain leaf block immediately
                Broadcasts the received block to its neighbors

NOTE - Check if any of the condition of releasing chain is met. If yes, release the chain
*/
void MaliciousPeer::receiveBlock(int sender_id, string block)
{
    pair<string, Block *> block_pair = parse_block(block);
    string hash_val = block_pair.first;
    Block *block_ptr = block_pair.second;

    if (seen_blocks.find(hash_val) != seen_blocks.end())
    {
        return; // block already received
    }

    if (pending_requests.find(hash_val) != pending_requests.end())
        pending_requests.erase(hash_val);
    if (timeouts.find(hash_val) != timeouts.end())
        timeouts.erase(hash_val);

    int blk_id = block_ptr->BlkID;
    string parent_hash = block_ptr->parent_hash;
    vector<string> transactions = block_ptr->txns;
    seen_blocks[hash_val] = block_ptr;

    if (blockTree.find(parent_hash) != blockTree.end())
    {
        int arrivalTime = curr_time;
        int parent_id = seen_blocks[parent_hash]->BlkID;
        timeline[arrivalTime].push_back({blk_id, parent_id});
        map<int, int> balances_temp;
        if (validateBlock(block_ptr, balances_temp))
        {

            if (block_ptr->miner_id == malNet->ringmasterID)
            {
                
                valid_timeline[arrivalTime].push_back({blk_id, parent_id, 1});
            }
            else
            {
                valid_timeline[arrivalTime].push_back({blk_id, parent_id, 0});
            }
            treeNode *parentNode = blockTree.at(parent_hash);
            treeNode *child = new treeNode(parentNode, blk_id, peerID);
            child->hash = hash_val;
            child->balances = balances_temp;
            if (hash_val == malNet->attack_root)
            {
                longestMalChain = hash_val;
                longestChain = hash_val;
            }
            blockTree[hash_val] = child;
            if (malNet->malicious_peers.find(block_ptr->miner_id) == malNet->malicious_peers.end())
            // miner was honest
            {
                broadcastHash(hash_val, 0);
                leafBlocks.erase(parent_hash);
                leafBlocks.insert(hash_val);
                if (child->depth > maxDepth)
                {

                    maxDepth = child->depth;
                    longestChain = hash_val;

                    if (peerID == malNet->ringmasterID)
                    {
                        if (maxDepth == malNet->malicious_len || maxDepth == malNet->malicious_len - 1)
                        {
                            startReleasingChain();
                        }
                        else if (maxDepth > malNet->malicious_len)
                        {
                            malNet->attack_root = longestChain;
                            malNet->malicious_len = 0;
                            malNet->malicious_leaf = longestChain;

                            for (auto malPeerID : malNet->malicious_peers)
                            {
                                if (malNet->peers[malPeerID]->blockTree.find(longestChain) != malNet->peers[malPeerID]->blockTree.end())
                                {
                                    malNet->peers[malPeerID]->longestMalChain = longestChain;
                                    malNet->peers[malPeerID]->longestChain = longestChain;
                                }
                            }
                        }
                    }
                    for (string txn : transactions)
                    {
                        memPool.erase(txn);
                    }
                    generateBlock();
                }
                processOrphanBlocks(hash_val);
            }
            else
            {
                broadcastHash(hash_val, 1);
                longestMalChain = hash_val;
                processOrphanBlocks(hash_val);
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
void HonestPeer::processOrphanBlocks(string hash_val)
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
                valid_timeline[curr_time].push_back({seen_blocks[orphan]->BlkID, block->BlkID, block->miner_id, 0});
                treeNode *parentNode = blockTree.at(hash_val);
                treeNode *orphanChild = new treeNode(parentNode, seen_blocks[orphan]->BlkID, peerID);
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
                broadcastHash(orphan, 0);
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
}

/*
Given a block, checks is any of the orphan block's are children of this block, if yes
process accordingly
*/
void MaliciousPeer::processOrphanBlocks(string hash_val)
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
                if (seen_blocks[orphan]->miner_id == malNet->ringmasterID)
                {
                    
                    valid_timeline[curr_time].push_back({seen_blocks[orphan]->BlkID, block->BlkID, block->miner_id, 1});
                }
                else
                {

                    valid_timeline[curr_time].push_back({seen_blocks[orphan]->BlkID, block->BlkID, block->miner_id, 0});
                }
                treeNode *parentNode = blockTree.at(hash_val);
                treeNode *orphanChild = new treeNode(parentNode, seen_blocks[orphan]->BlkID, peerID);
                orphanChild->hash = orphan;
                orphanChild->balances = balances_temp;
                if (orphan == malNet->attack_root)
                {
                    longestMalChain = orphan;
                    longestChain = orphan;
                }
                blockTree[orphan] = orphanChild;

                if (malNet->malicious_peers.find(seen_blocks[orphan]->miner_id) == malNet->malicious_peers.end())
                // miner was honest
                {

                    leafBlocks.erase(hash_val);
                    leafBlocks.insert(orphan);
                    if (orphanChild->depth > maxDepth)
                    {
                        maxDepth = orphanChild->depth;
                        longestChain = orphan;

                        if (peerID == malNet->ringmasterID)
                        {
                            if (maxDepth == malNet->malicious_len || maxDepth == malNet->malicious_len - 1)
                            {
                                startReleasingChain();
                            }
                            else if (maxDepth > malNet->malicious_len)
                            {
                                malNet->attack_root = longestChain;
                                malNet->malicious_len = 0;
                                malNet->malicious_leaf = longestChain;

                                for (auto malPeerID : malNet->malicious_peers)
                                {
                                    if (malNet->peers[malPeerID]->blockTree.find(longestChain) != malNet->peers[malPeerID]->blockTree.end())
                                    {
                                        malNet->peers[malPeerID]->longestMalChain = longestChain;
                                        malNet->peers[malPeerID]->longestChain = longestChain;
                                    }
                                }
                            }
                        }

                        for (string txn : seen_blocks[orphan]->txns)
                        {
                            memPool.erase(txn);
                        }
                    }
                    (orphan, 0);
                    generateBlock();
                    processOrphanBlocks(orphan);
                }

                else
                // ringmaster
                {
                    broadcastHash(orphan, 1);
                    longestMalChain = orphan;
                    processOrphanBlocks(orphan);
                }
            }
            toRemove.push_back(orphan);
        }
    }
    for (string orphan : toRemove)
    {
        orphanBlocks.erase(orphan);
    }
}

/*
Validates the blocks -> checks if the sender of every transaction included has sufficient balance
*/
bool Peer::validateBlock(Block *block, map<int, int> &balances_temp)
{
    treeNode *parent = blockTree.at(block->parent_hash);
    map<int, int> balances = parent->balances;

    int ctr = 0;

    for (auto txn : block->txns)
    {
        vector<int> txn_vec = parse_txn(txn);
        // txn id, x pays y amt
        if (txn_vec[1] == -1)
        {
            balances[txn_vec[2]] += 50;
            continue;
        }
        if (balances[txn_vec[1]] >= txn_vec[3])
        {
            balances[txn_vec[1]] -= txn_vec[3];
            balances[txn_vec[2]] += txn_vec[3];
        }
        else
        {
            return false;
        }
        ctr++;
    }
    balances[block->miner_id] += 50;
    balances_temp = balances;
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

        outFile << "BlockID : Time : Parent BlockID : Honest/ Malicious " << endl;
        outFile << "---------------------------------" << endl;
        outFile << "0 : 0 : -1 : -1" << endl;

        for (const auto &entry : valid_timeline)
        {
            for (const auto &id : entry.second)
            {
                outFile << id[0] << " : " << entry.first << " : " << id[1] << " : " << id[2] << "\n";
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
    seen_blocks[hash_val] = genesis_block;
    blockTree[hash_val] = genesis_blk;
    longestChain = hash_val;
    longestMalChain = hash_val;
    malNet->malicious_len = 0;
    leafBlocks.insert(hash_val);
    if (peerID == malNet->ringmasterID)
    {
        malNet->attack_root = hash_val;
        malNet->malicious_leaf = hash_val;
    }
}

/* Helper function to determine whether or not to forward a message */
bool Peer::query(int txn_id)
{
    if (transactionSet.find(txn_id) != transactionSet.end())
        return false;
    transactionSet.insert(txn_id);
    return true;
}

pair<int, int> HonestPeer::blocks_in_longest_chain()
{
    // int num_blocks = 0;

    // // Traverse backwards to find which nodes contributed blocks to the longest chain
    // treeNode *currNode = blockTree.at(longestChain);
    // while (currNode->block_id != 0)
    // {
    //     string hash = currNode->hash;
    //     if (seen_blocks[hash]->miner_id == peerID)
    //     {
    //         num_blocks++;
    //     }

    //     currNode = blockTree.at(currNode->parent_hash);
    // }

    // return num_blocks;
    return {0,0};
}


pair<int, int> MaliciousPeer::blocks_in_longest_chain()
{
    int ringmaster_blocks = 0;
    int total_blocks = 0;

    // Traverse backwards to find which nodes contributed blocks to the longest chain
    treeNode *currNode = blockTree.at(malNet->malicious_leaf);
    while (currNode->block_id != 0)
    {
        string hash = currNode->hash;
        if (seen_blocks[hash]->miner_id == malNet->ringmasterID)
        {
            ringmaster_blocks++;
        }
        total_blocks++;
        currNode = blockTree.at(currNode->parent_hash);
    }

    return {ringmaster_blocks, total_blocks};
}

/*
Called by honest peer to add its own mined block to its tree
*/
void HonestPeer::addBlocktoTree(string hash_val)
{
    Block *block = seen_blocks[hash_val];
    treeNode *parentNode = blockTree.at(block->parent_hash);
    treeNode *child = new treeNode(parentNode, block->BlkID, peerID);
    child->hash = hash_val;

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

/*
Called by malicious peer to add its own mined block to its tree
*/
void MaliciousPeer::addBlocktoTree(string hash_val)
{
    Block *block = seen_blocks[hash_val];
    treeNode *parentNode = blockTree.at(block->parent_hash);
    treeNode *child = new treeNode(parentNode, block->BlkID, peerID);
    child->hash = hash_val;
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
    if (peerID == malNet->ringmasterID)
    {
        malNet->malicious_len++;
        malNet->malicious_leaf = hash_val;
    }
    for (string txn : block->txns)
    {
        memPool2.erase(txn);
    }
}

void HonestPeer::releaseChain(string)
{
    return;
}

void HonestPeer::startReleasingChain()
{
    return;
}

/*
Called by the ringmaster of overlay network to instruct
malicious peers to start releasing the private chain
*/
void MaliciousPeer::startReleasingChain()
{
    if (peerID != malNet->ringmasterID)
        return;
    string message = "broadcast private chain";
    double messagesize = 0.064;
    message += "%" + malNet->attack_root;
    message += "%" + malNet->malicious_leaf;

    for (auto receiver : malicious_neighbours)
    {
        int lt = malNet->calculateLatency(peerID, receiver, messagesize);
        malNet->sendingQueue.push({{curr_time + lt, 4, peerID, receiver}, message});
    }

    string curr_mal_leaf = malNet->malicious_leaf;
    string curr_attack_root = malNet->attack_root;

    string curr_hash = curr_mal_leaf;
    while (curr_hash != curr_attack_root)
    {
        blocks_to_release.insert(curr_hash);
        broadcastHash(curr_hash, 0);
        curr_hash = blockTree[curr_hash]->parent_hash;
    }

    malNet->attack_root = malNet->malicious_leaf;
    malNet->malicious_len = 0;

    for (auto malPeerID : malNet->malicious_peers)
    {
        if (malNet->peers[malPeerID]->seen_blocks.find(malNet->malicious_leaf) != malNet->peers[malPeerID]->seen_blocks.end())
        {
            malNet->peers[malPeerID]->longestMalChain = malNet->malicious_leaf;
            malNet->peers[malPeerID]->longestChain = malNet->malicious_leaf;
        }
    }
}

/*
Called by the malicious peers to release the private chain onto the original network
*/
void MaliciousPeer::releaseChain(string message)
{
    pair<string, string> p = extract_root(message);
    string curr_attack_root = p.first;
    string curr_mal_leaf = p.second;

    if (released_chains.find(curr_attack_root) != released_chains.end())
    {
        return;
    }
    released_chains.insert(curr_attack_root);

    double messagesize = 0.064;
    for (auto receiver : malicious_neighbours)
    {

        int lt = malNet->calculateLatency(peerID, receiver, messagesize);
        malNet->sendingQueue.push({{curr_time + lt, 4, peerID, receiver}, message});
    }

    string curr_hash = curr_mal_leaf;
    while (curr_hash != curr_attack_root)
    {
        blocks_to_release.insert(curr_hash);
        broadcastHash(curr_hash, 0);
        curr_hash = blockTree[curr_hash]->parent_hash;
    }
}