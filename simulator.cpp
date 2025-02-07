#include "simulator.h"
#include <random>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace std;

int num_txns = 0;
int time_stamp = 0;




void P2P::start()
{
    for (int i = 0; i < num_peers; i++)
    {
        peers[i].generateBlock();
        peers[i].generateTransaction();
    }

    while (1)
    {
        auto next_blk = blockQueue.top();
        auto next_txn = transactionQueue.top();
        auto next_msg = sendingQueue.top();
        if (next_blk[0] <= next_msg[0] && next_blk[0] <= next_txn[0])
        {
            blockQueue.pop();
            int sender = next_blk[2];
            int blkid = next_blk[1];
            if (peers[sender].longestChain == globalBlocks[blkid]->parent_id)
            {
                peers[sender].broadcastBlock(blkid);
                peers[sender].generateBlock();
            }
            else
            {
                delete globalBlocks[blkid];
                globalBlocks[blkid] = NULL;
            }
        }
        else if (next_txn[0] <= next_msg[0] && next_txn[0] <= next_blk[0])
        {
            transactionQueue.pop();
            peers[next_txn[1]].broadcastTransaction();
        }
        else
        {
            sendingQueue.pop();
            if (next_msg[1] == 0)
            {
                peers[next_msg[3]].receiveTransaction(next_msg[2]);
            }
            else
            {
                // handle rcving block
            }
        }
    }
}

int main(int argc,char** argv){
    if(argc != 6){
        cout << "USAGE: ./{executable} {slow nodes} {slow CPU nodes} {number of peers} {simulation time} {I} {Ttx}" <<endl;
    }
    P2P *simulator = new P2P();
    simulator->z0 = stoi(argv[1]);
    simulator->z1 = stoi(argv[2]);
    simulator->numPeers = stoi(argv[3]);
    simulator->I = stoi(argv[5]);
    simulator->Ttx = stoi(argv[6]);
    simulator->start();
}
