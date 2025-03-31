#include "structure.h"

Peer::Peer()
{
    malicious_neighbours = {};
}

Peer::~Peer()
{
    for (auto it : blockTree)
    {
        delete it.second;
    }
    delete normNet;
    delete genesis_blk;
}

HonestPeer::HonestPeer(int pID, Network *normNet)
{
    this->normNet = normNet;
    peerID = pID;
    total_blocks = 0;
    total_transactions = 0;
    failed_txns = 0;
    maxDepth = 0;
    longestChain = "";
    slow = true;
}

MaliciousPeer::MaliciousPeer(int pID, int mID, Network *normNet, OverlayNetwork *malNet)
{
    this->normNet = normNet;
    this->malNet = malNet;
    peerID = pID;
    total_blocks = 0;
    total_transactions = 0;
    failed_txns = 0;
    maxDepth = 0;
    longestChain = "";
    slow = false;
    released_chains = {};
    blocks_to_release = {};
    longestMalChain = "";
}

Block::Block(int id, int miner_id, string parent_hash, vector<string> txns)
{
    this->miner_id = miner_id;
    this->parent_hash = parent_hash;
    this->txns = txns;
    this->BlkID = id;
}

treeNode::treeNode(treeNode *parent_node, int id, int peerID)
{
    this->depth = parent_node ? parent_node->depth + 1 : 0;
    this->parent_hash = parent_node ? parent_node->hash : "parent_of_genesis";
    this->block_id = id;
    this->peerID = peerID;
    this->balances = parent_node ? parent_node->balances : map<int, int>();
}