#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

void generateGraphvizDotFile(const std::string &inputFile, const std::string &outputDotFile, const std::string &outputPngFile) {
    std::ifstream inFile(inputFile);
    if (!inFile) {
        std::cerr << "Error opening input file: " << inputFile << std::endl;
        return;
    }

    std::ofstream outFile(outputDotFile);
    if (!outFile) {
        std::cerr << "Error opening output file: " << outputDotFile << std::endl;
        return;
    }

    outFile << "digraph BlockchainTree {\n";
    outFile << "    rankdir=LR;\n";  // Set horizontal layout
    outFile << "    node [shape=circle, style=filled, fillcolor=lightgray];\n\n";

    std::string line;
    bool firstLine = true;

    while (std::getline(inFile, line)) {
        if (firstLine || line.find("---") != std::string::npos) {
            firstLine = false;
            continue; // Skip header and separator line
        }

        std::istringstream iss(line);
        int blockID, time, parentID;
        char colon1, colon2;

        if (!(iss >> blockID >> colon1 >> time >> colon2 >> parentID)) {
            continue; // Skip malformed lines
        }

        if (parentID == -1) {
            outFile << "    " << blockID << " [shape=doublecircle, color=red];\n"; // Genesis block
        } else {
            outFile << "    " << parentID << " -> " << blockID << ";\n";
        }
    }

    outFile << "}\n";
    outFile.close();
    inFile.close();

    // Generate PNG from DOT using Graphviz
    std::string command = "dot -Tpng " + outputDotFile + " -o " + outputPngFile;
    system(command.c_str());
    std::cout << "Generated: " << outputPngFile << std::endl;
}

int main() {
    std::string outputDir = "output"; // Folder where all txt files are stored

    for (const auto &entry : fs::directory_iterator(outputDir)) {
        if (entry.path().extension() == ".txt" && entry.path().filename().string().find("block_times_") != std::string::npos) {
            std::string inputFile = entry.path().string();
            std::string peerID = entry.path().stem().string().substr(12); // Extract peer number

            std::string outputDotFile = "output/dot/block_tree_" + peerID + ".dot";
            std::string outputPngFile = "output/pic/block_tree_" + peerID + ".png";

            generateGraphvizDotFile(inputFile, outputDotFile, outputPngFile);
        }
    }

    return 0;
}
