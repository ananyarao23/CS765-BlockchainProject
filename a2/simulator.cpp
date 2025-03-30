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
    
    genesisBlock= new Block(0, -1, "", {});
    for (auto& peer : peers)
    {
        peer->createTree(genesisBlock);
        peer->generateBlock();
        peer->generateTransaction();
        cout<<"Peer "<<peer->peerID<<" tree created"<<endl;
    }
    cout<<"xotwod"<<endl;
    while (curr_time < simTime)
    {
        
        cout << "TIMESTAMP" << " " << curr_time << endl;
        normNet->run(curr_time);
        malNet->run(curr_time);
        curr_time++;
    }

    // Discrete event simulator
    // while (curr_time < simTime)
    // {
    //     if(MalNet->mined_length == NormNet->mined_length)
    //     {
    //         // malicious nodes broadcast the new attacker's chain

    //     }
    //     else if(MalNet->mined_length == NormNet->mined_length - 1)
    //     {
    //         // malicious nodes broadcast the new attacker's chain
    //     }
    //     else
    //     {
    //         // honest and overlay broadcast continues

    //     }
    // }
}

int main(int argc, char **argv)
{
    if (argc != 7)
    {
        cout << "USAGE: ./{executable} {number of peers} {percentage of malicious nodes} {I} {Ttx} {Timeout time} {simulation time}" << endl;
        return 1;
    }
    numPeers = stoi(argv[1]);
    I = stof(argv[3]);
    Ttx = stof(argv[4]);
    Tt = stof(argv[5]);
    // numpeers malpercent simtime
    Sim *simulator = new Sim(stoi(argv[1]), double(stoi(argv[2]))/100, stoi(argv[6]) * 1000);
    cout << "-----------------------------------------Starting the simulation-----------------------------------------" << endl;
    simulator->start();

    cout << setw(7) << "Peer ID" << setw(7) << "Speed" << setw(7) << "CPU" << setw(32) << "Blocks owned in Longest Chain" << setw(15) << "Blocks mined" << setw(10) << "Ratio" << setw(25) << "Blocks in Longest Chain" << setw(25) << "Transactions generated" << endl;
    cout << string(130, '-') << endl;
    // for (auto p : simulator->peers)
    // {
    //     Peer peer = *p;
    //     cout << setw(7) << peer.peerID
        // << setw(32) << peer.blocks_in_longest_chain()
    //     << setw(15) << int(peer.total_blocks)
    //     << setw(10) << fixed << setprecision(2) << (peer.total_blocks > 0 ? static_cast<double>(peer.blocks_in_longest_chain()) / peer.total_blocks : 0)
    //     << setw(25) << peer.maxDepth
    //     << setw(25) << peer.total_transactions
    //     << endl;

    //     peer.writeBlockTimesToFile();
    // }
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
