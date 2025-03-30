#include "peer.h"
#include "simulator.h"
#include <iomanip>

using namespace std;

int num_txns = 0;
int time_stamp = 0;

int txnIDctr = 0;            // for setting txn ID
int blkIDctr = 1;            // for setting block ID
long long int curr_time = 0; // time counter
float Tt = 0;
float I = 0;
float Ttx = 0;
int numPeers = 0;
Block* genesisBlock;

/* Starts the simulation */
void Sim::start()
{
    srand(time(0));    
    genesisBlock= new Block(0, -1, "parent_of_genesis", {});
    for (auto& peer : peers)
    {
        peer->createTree(genesisBlock);
        peer->generateBlock();
        peer->generateTransaction();
        cout<<"Peer "<<peer->peerID<<" tree created"<<endl;
    }
    
    while (curr_time < simTime)
    {
        
        normNet->run(curr_time);
        if (malFraction != 0)  malNet->run(curr_time);
        curr_time++;
    }

}

int main(int argc, char **argv)
{
    if (argc != 7)
    {
        cout << "USAGE: ./{executable} {number of peers} {percentage of malicious nodes} {I} {Ttx} {Timeout time} {simulation time}" << endl;
        return 1;
    }
    numPeers = stoi(argv[1]);
    I = stof(argv[3])/1000;
    Ttx = stof(argv[4]);
    Tt = stof(argv[5]);
    // numpeers malpercent simtime
    Sim *simulator = new Sim(stoi(argv[1]), double(stoi(argv[2]))/100, stoi(argv[6]));
    cout << "-----------------------------------------Starting the simulation-----------------------------------------" << endl;
    simulator->start();

    cout << setw(7) << "Peer ID" << setw(32) << "Blocks owned in Longest Chain" << setw(15) << "Blocks mined" << setw(10) << "Ratio" << setw(25) << "Blocks in Longest Chain" << setw(25) << "Transactions generated" << "orphan blks"<<endl;
    cout << string(130, '-') << endl;
    for (auto p : simulator->peers)
    {
        cout << setw(7) << p->peerID
        << setw(32) << p->blocks_in_longest_chain()
        << setw(15) << int(p->total_blocks)
        << setw(10) << fixed << setprecision(2) << (p->total_blocks > 0 ? static_cast<double>(p->blocks_in_longest_chain()) / p->total_blocks : 0)
        << setw(25) << p->maxDepth
        << setw(25) << p->total_transactions
        << setw(25) << p->orphanBlocks.size()
        << endl;

        p->writeBlockTimesToFile();
    }
    // cout<<simulator->peers[simulator->malNet->ringmasterID]->malicious_len<<endl;
    
    cout << "------------------------------------------------Simulation ended------------------------------------------" << endl;

    // ofstream outFile("simulation_results.csv");
    // outFile << "PeerID,Speed,CPU,BlocksOwned,BlocksMined,Ratio,MaxDepth,TransactionsGenerated\n";

    // for (auto peer : simulator->peers)
    // {
    //     outFile << peer->peerID << ","
    //             << (peer->slow ? "Slow" : "Fast") << ","
    //             << peer->blocks_in_longest_chain() << ","
    //             << peer->total_blocks << ","
    //             // << (peer->total_blocks > 0 ? static_cast<double>(peer.blocks_in_longest_chain()) / peer.total_blocks : 0) << ","
    //             << peer->maxDepth << ","
    //             << peer->total_transactions << "\n";
    // }
    // outFile.close();

    return 0;
}
