/* For visualization and result compilation */
void Peer::writeBlockTimesToFile()
{
    filesystem::create_directories("output/tree");  // Ensure directory exists
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
void Peer::createTree(Block *genesis_block)
{
    genesis_blk = new treeNode(nullptr, genesis_block);
    genesis_blk->depth = 0;
    genesis_blk->parent_ptr = nullptr;
    genesis_blk->block_id = 0;
    blockTree[0] = genesis_blk;
    blockSet.insert(0);
    longestChain = 0;
    leafBlocks.insert(0);
}

/* Helper function to determine whether or not to forward a message */
bool Peer::query(int txn_id)
{
    if (transactionSet.find(txn_id) != transactionSet.end())
        return false;
    transactionSet.insert(txn_id);
    return true;
}

void Peer::treeAnalysis()
{
    map<int, bool> inBlockChain;
    int blkid = longestChain;
    while (blkid != 0)
    {
        inBlockChain[blkid] = true;
        blkid = globalBlocks[blkid]->parent_id;
    }
    inBlockChain[0] = true;

    vector<int> branch_len;
    for (auto leaf : leafBlocks)
    {
        int len = 0;
        while (!inBlockChain[leaf])
        {
            len++;
            leaf = globalBlocks[leaf]->parent_id;
        }
        branch_len.push_back(len);
    }
}

int Peer::blocks_in_longest_chain()
{
    int num_blocks = 0;

    // Traverse backwards to find which nodes contributed blocks to the longest chain
    treeNode *currNode = blockTree[longestChain];
    while (currNode->block_id != 0)
    {
        int bID = currNode->block_id;
        if (globalBlocks[bID]->miner_id == peerID)
        {
            num_blocks++;
        }
        currNode = currNode->parent_ptr;
    }

    return num_blocks;
}

void Peer::addBlocktoTree(int blkid)
{
    Block *block = globalBlocks[blkid];
    treeNode *parentNode = blockTree[block->parent_id];
    treeNode *child = new treeNode(parentNode, block);
    child->balances = parentNode->balances;

    for (int txn : block->txns)
    {
        Transaction *tx = globalTransactions[txn];
        child->balances[tx->sender_id] -= tx->amount;
        child->balances[tx->receiver_id] += tx->amount;
    }

    child->balances[peerID] += 50;
    blockTree[blkid] = child;
    blockSet.insert(blkid);
    maxDepth = child->depth;
    longestChain = blkid;
    leafBlocks.erase(block->parent_id);
    leafBlocks.insert(blkid);

    for (int txn : block->txns)
    {
        memPool.erase(txn);
    }
}