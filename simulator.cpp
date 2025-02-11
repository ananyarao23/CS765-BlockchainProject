#include "simulator.h"
#include <fstream>
#include <iomanip> 
#include <filesystem>

using namespace std;

int num_txns = 0;
int time_stamp = 0;

int txnIDctr = 0;  // for setting txn ID
int blkIDctr = 1;  // for setting block ID
long long int curr_time = 0; // time counter
priority_queue<vector<int>, vector<vector<int>>, Compare> sendingQueue;
priority_queue<vector<int>, vector<vector<int>>, Compare> transactionQueue;
priority_queue<vector<int>, vector<vector<int>>, Compare> blockQueue;

unordered_map<int, Block *> globalBlocks;             // maps block ID to block ptr
unordered_map<int, Transaction *> globalTransactions; // maps transaction ID to transaction ptr

/* Sets the hash power for the peer */
void Peer::setHashPower(double x)
{
    hash_power = x;
}

/* Sets the CPU category for the peer */
void Peer::setlowCPU()
{
    lowCPU = true;
}

/* Sets the speed category for the peer */
void Peer::setslow()
{
    slow = true;
}

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

/* Randomly sets a fraction of the P2P peers to be slow */
void P2P::assignSlowFast()
{
    int num = z0 * numPeers / 100;
    vector<int> indices = randomIndices(num, numPeers);
    for (int i : indices)
    {
        peers[i].setslow();
    }
}

/* Randomly sets a fraction of the P2P peers to have low CPU power */
void P2P::assignCPU()
{
    int num = z1 * numPeers / 100;
    vector<int> indices = randomIndices(num, numPeers);
    for (int i : indices)
    {
        peers[i].setlowCPU();
    }
}

/* Computes the fraction of hash power for low and high CPU peers */
void P2P::computeHashPower()
{
    double x;
    double coefficient = 0;
    for (int i = 0; i < numPeers; i++)
    {
        if (peers[i].lowCPU)
            coefficient += 1;
        else
            coefficient += 10;
    }
    x = 1.0 / coefficient;
    for (int i = 0; i < numPeers; i++)
    {
        if (peers[i].lowCPU)
            peers[i].setHashPower(x);
        else
            peers[i].setHashPower(x * 10);
    }
}

/* Samples propagation delays from a uniform distribution */
void P2P::assignPropDelay()
{
    for (int i = 0; i < numPeers; i++)
    {
        prop_delay.push_back(vector<double>(numPeers));
        for (int j = 0; j < numPeers; j++)
        {
            prop_delay[i][j] = sampleUniform(0.01, 0.5);
        }
    }
}

/* Assign link speeds based on both nodes */
void P2P::assignLinkSpeed()
{
    for (int i = 0; i < numPeers; i++)
    {
        link_speed.push_back(vector<double>(numPeers));
        for (int j = 0; j < numPeers; j++)
        {
            link_speed[i][j] = peers[i].slow || peers[j].slow ? 5 : 100;
        }
    }
}

/* Returns the latency (in ms) gives peer IDs and message size */
int P2P::calculateLatency(int i, int j, double ms)
{
    double p = prop_delay[i][j] * 1000;  // ms
    double c = link_speed[i][j];         // Mbps
    int d = generateExponential(96 / c); // ms
    // ms in kilobits
    int latency = p + d + ms / c;
    return latency;
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

/* Starts the simulation */
void P2P::start()
{
    Block *genesis_block = new Block(0, -1, {});
    globalBlocks[genesis_block->BlkID] = genesis_block;
    for (int i = 0; i < numPeers; i++)
    {
        peers[i].createTree(genesis_block);
        peers[i].generateBlock();
        peers[i].generateTransaction();
    }

    // Discrete event simulator
    while (curr_time < simTime)
    {
        vector<int> next_blk, next_msg, next_txn;

        if (!blockQueue.empty())
        {
            next_blk = blockQueue.top();
        }

        while (!blockQueue.empty() && next_blk[0] == curr_time)

        { // a block is ready to be broadcasted by its miner
            blockQueue.pop();
            int sender = next_blk[2];
            int blkid = next_blk[1];
            if (peers[sender].longestChain == globalBlocks[blkid]->parent_id)
            {
                peers[sender].total_blocks++;
                peers[sender].addBlocktoTree(blkid);
                peers[sender].timeline[curr_time].push_back({blkid, globalBlocks[blkid]->parent_id});
                peers[sender].valid_timeline[curr_time].push_back({blkid, globalBlocks[blkid]->parent_id});
                peers[sender].broadcastBlock(blkid);
                peers[sender].generateBlock();
            }
            else
            {
                delete globalBlocks[blkid];
                globalBlocks[blkid] = NULL;
            }
            if (!blockQueue.empty())
                next_blk = blockQueue.top();
        }
        if (!transactionQueue.empty())
        {
            next_txn = transactionQueue.top();
        }
        while (!transactionQueue.empty() && next_txn[0] == curr_time)

        {
            // a transaction is ready to be broadcasted by its creator
            total_transactions++;
            transactionQueue.pop();
            peers[next_txn[1]].broadcastTransaction();
            if (!transactionQueue.empty())
                next_txn = transactionQueue.top();
        }
        if (!sendingQueue.empty())
        {
            next_msg = sendingQueue.top();
        }
        while (!sendingQueue.empty() && next_msg[0] == curr_time)

        { // a message (block or transaction) is ready to be received by a peer (targeted receiver)
            sendingQueue.pop();
            if (next_msg[1] == 0)
            {
                peers[next_msg[3]].receiveTransaction(next_msg[2]);
            }
            else
            {
                peers[next_msg[3]].receiveBlock(next_msg[2]);
            }
            if (!sendingQueue.empty())
                next_msg = sendingQueue.top();
        }
        curr_time++;
    }
}

int main(int argc, char **argv)
{
    if (argc != 7)
    {
        cout << "USAGE: ./{executable} {slow nodes} {slow CPU nodes} {number of peers} {simulation time} {I} {Ttx}" << endl;
        return 1;
    }

    P2P *simulator = new P2P(stoi(argv[1]), stoi(argv[2]), stoi(argv[3]), stoi(argv[4])*1000, stoi(argv[5]), stoi(argv[6])*1000);
    cout << "-----------------------------------------Starting the simulation-----------------------------------------" << endl;
    simulator->start();

    cout << setw(7) << "Peer ID" << setw(7) << "Speed" << setw(7) << "CPU" << setw(32) << "Blocks owned in Longest Chain" <<  setw(15) << "Blocks mined" << setw(10) << "Ratio" << setw(25) << "Blocks in Longest Chain" << setw(25) << "Transactions generated" << endl;
    cout << string(130, '-') << endl;
    for (auto peer : simulator->peers)
    {
        cout << setw(7) << peer.peerID
             << setw(7) << (peer.slow ? "Slow" : "Fast")
             << setw(7) << (peer.lowCPU ? "Low" : "High")
             << setw(32) << peer.blocks_in_longest_chain()
             << setw(15) << int(peer.total_blocks)
             << setw(10) << fixed << setprecision(2) << (peer.total_blocks > 0 ? static_cast<double>(peer.blocks_in_longest_chain()) / peer.total_blocks : 0)
             << setw(25) << peer.maxDepth
             << setw(25) << peer.total_transactions
             << endl;

        peer.writeBlockTimesToFile();
    }
    cout << "------------------------------------------------Simulation ended------------------------------------------" << endl;

    ofstream outFile("simulation_results.csv");
    outFile << "PeerID,Speed,CPU,BlocksOwned,BlocksMined,Ratio,MaxDepth,TransactionsGenerated\n";
    
    for (auto peer : simulator->peers)
    {
        outFile << peer.peerID << ","
                << (peer.slow ? "Slow" : "Fast") << ","
                << (peer.lowCPU ? "Low" : "High") << ","
                << peer.blocks_in_longest_chain() << ","
                << peer.total_blocks << ","
                << (peer.total_blocks > 0 ? static_cast<double>(peer.blocks_in_longest_chain()) / peer.total_blocks : 0) << ","
                << peer.maxDepth << ","
                << peer.total_transactions << "\n";
    }
    outFile.close();

    return 0;
}
