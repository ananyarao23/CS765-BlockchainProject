// #include <fstream>
// #include <iomanip> 
// #include <filesystem>


// /* For visualization and result compilation */
// // void Peer::writeBlockTimesToFile()
// // {
// //     filesystem::create_directories("output/tree");  // Ensure directory exists
// //     filesystem::create_directories("output/valid_tree");
    
// //     // Writing to "output/tree/block_times_<peerID>.txt"
// //     {
// //         ofstream outFile("output/tree/block_times_" + to_string(peerID) + ".txt");
// //         if (!outFile.is_open())
// //         {
// //             cerr << "Error: Could not open output/tree file for writing\n";
// //             return;
// //         }
        
// //         outFile << "BlockID : Time: Parent BlockID " << endl;
// //         outFile << "---------------------------------" << endl;
// //         outFile << "0 : 0 : -1" << endl;

// //         for (const auto &entry : timeline)
// //         {
// //             for (const auto &id : entry.second)
// //             {
// //                 outFile << id.first << " : " << entry.first << " : " << id.second << "\n";
// //             }
// //         }
// //         outFile.close();
// //     }
    
// //     // Writing to "output/valid_tree/block_times_<peerID>.txt"
// //     {
// //         ofstream outFile("output/valid_tree/block_times_" + to_string(peerID) + ".txt");
// //         if (!outFile.is_open())
// //         {
// //             cerr << "Error: Could not open output/valid_tree file for writing\n";
// //             return;
// //         }

// //         outFile << "BlockID : Time: Parent BlockID " << endl;
// //         outFile << "---------------------------------" << endl;
// //         outFile << "0 : 0 : -1" << endl;

// //         for (const auto &entry : valid_timeline)
// //         {
// //             for (const auto &id : entry.second)
// //             {
// //                 outFile << id.first << " : " << entry.first << " : " << id.second << "\n";
// //             }
// //         }
// //         outFile.close();
// //     }
// // }


// /* Takes as argument the global genesis block pointer and initializes the tree */
// void Peer::createTree(Block *genesis_block)
// {
//     genesis_blk = new treeNode(nullptr, genesis_block->BlkID);
//     genesis_blk->depth = 0;
//     genesis_blk->parent_hash = "";
//     genesis_blk->block_id = 0;
//     string hash_val = calculateHash(0,-1,"",{});
//     blockTree[hash_val] = genesis_blk;
//     longestChain = hash_val;
//     leafBlocks.insert(hash_val);
// }

// /* Helper function to determine whether or not to forward a message */
// bool Peer::query(int txn_id)
// {
//     if (transactionSet.find(txn_id) != transactionSet.end())
//         return false;
//     transactionSet.insert(txn_id);
//     return true;
// }

// // void Peer::treeAnalysis()
// // {
// //     map<int, bool> inBlockChain;
// //     int blkid = longestChain;
// //     while (blkid != 0)
// //     {
// //         inBlockChain[blkid] = true;
// //         blkid = globalBlocks[blkid]->parent_id;
// //     }
// //     inBlockChain[0] = true;

// //     vector<int> branch_len;
// //     for (auto leaf : leafBlocks)
// //     {
// //         int len = 0;
// //         while (!inBlockChain[leaf])
// //         {
// //             len++;
// //             leaf = globalBlocks[leaf]->parent_id;
// //         }
// //         branch_len.push_back(len);
// //     }
// // }

// // int Peer::blocks_in_longest_chain()
// // {
// //     int num_blocks = 0;

// //     // Traverse backwards to find which nodes contributed blocks to the longest chain
// //     treeNode *currNode = blockTree[longestChain];
// //     while (currNode->block_id != 0)
// //     {
// //         int bID = currNode->block_id;
// //         if (globalBlocks[bID]->miner_id == peerID)
// //         {
// //             num_blocks++;
// //         }
// //         currNode = currNode->parent_ptr;
// //     }

// //     return num_blocks;
// // }

// void Peer::addBlocktoTree(string hash_val)
// {
//     Block *block = seen_blocks[hash_val];
//     treeNode *parentNode = blockTree[block->parent_hash];
//     treeNode *child = new treeNode(parentNode, block->BlkID);
//     child->balances = parentNode->balances;

//     for (string txn : block->txns)
//     {
//         vector<int> txn_vec = parse_txn(txn);
//         child->balances[txn_vec[1]] -= txn_vec[3];
//         child->balances[txn_vec[2]] += txn_vec[3];
//     }

//     child->balances[peerID] += 50;
//     blockTree[hash_val] = child;
//     maxDepth = child->depth;
//     longestChain = hash_val;
//     leafBlocks.erase(block->parent_hash);
//     leafBlocks.insert(hash_val);

//     for (string txn : block->txns)
//     {
//         memPool.erase(txn);
//     }
// }

// void MaliciousPeer::addBlocktoTree(string hash_val)
// {
//     Block *block = seen_blocks[hash_val];
//     treeNode *parentNode = blockTree[block->parent_hash];
//     treeNode *child = new treeNode(parentNode, block->BlkID);
//     child->balances = parentNode->balances;

//     for (string txn : block->txns)
//     {
//         vector<int> txn_vec = parse_txn(txn);
//         child->balances[txn_vec[1]] -= txn_vec[3];
//         child->balances[txn_vec[2]] += txn_vec[3];
//     }

//     child->balances[peerID] += 50;
//     blockTree[hash_val] = child;
//     malicious_len++;
//     malicious_leaf = hash_val;
//     for (string txn : block->txns)
//     {
//         memPool2.erase(txn);
//     }
// }