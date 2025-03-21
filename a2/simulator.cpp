#include "structure.h"
#include <fstream>
#include <iomanip> 
#include <filesystem>
#include "network.h"
#include "network.cpp"
#include "peer.h"
#include "simulator.h"

using namespace std;

int num_txns = 0;
int time_stamp = 0;

int txnIDctr = 0;  // for setting txn ID
int blkIDctr = 1;  // for setting block ID
long long int curr_time = 0; // time counter


/* Starts the simulation */
void Sim:start()
{
    // Discrete event simulator
    while (curr_time < simTime)
    {
        P2P->run();
    }
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        cout << "USAGE: ./{executable} {number of peers} {percentage of malicious nodes} {I} {Ttx} {simulation time}" << endl;
        return 1;
    }

    Sim *simulator = new Sim(stoi(argv[1]), stoi(argv[2]), stoi(argv[3]), stof(argv[4])*1000, stoi(argv[5])*1000,);
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
