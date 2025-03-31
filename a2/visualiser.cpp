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

    fs::create_directories(fs::path(outputDotFile).parent_path());
    fs::create_directories(fs::path(outputPngFile).parent_path());

    std::ofstream outFile(outputDotFile);
    if (!outFile) {
        std::cerr << "Error opening output file: " << outputDotFile << std::endl;
        return;
    }

    outFile << "digraph BlockchainTree {\n";
    outFile << "    rankdir=LR;\n";  // Horizontal layout
    outFile << "    node [shape=circle, style=filled, fontcolor=black];\n\n";

    std::string line;
    bool firstLine = true;

    while (std::getline(inFile, line)) {
        if (firstLine || line.find("---") != std::string::npos) {
            firstLine = false;
            continue; // Skip header and separator line
        }

        std::istringstream iss(line);
        int blockID, time, parentID, status;
        char colon1, colon2, colon3;

        if (!(iss >> blockID >> colon1 >> time >> colon2 >> parentID >> colon3 >> status)) {
            continue; // Skip malformed lines
        }

        std::string color;
        if (status == 0) {
            color = "green";  // Honest block
        } else if (status == 1) {
            color = "red";    // Malicious block
        } else {
            color = "green";   // Undefined status
        }

        outFile << "    " << blockID << " [fillcolor=" << color << "];\n";

        if (parentID != -1) {
            outFile << "    " << parentID << " -> " << blockID << ";\n";
        } else {
            outFile << "    " << blockID << " [shape=doublecircle, color=blue];\n"; // Genesis block
        }
    }

    outFile << "}\n";
    outFile.close();
    inFile.close();

    std::string command = "dot -Tpng " + outputDotFile + " -o " + outputPngFile;
    system(command.c_str());
    std::cout << "Generated: " << outputPngFile << std::endl;
}

int main() {
    std::string outputDir = "output/valid_tree"; // Folder where all txt files are stored

    for (const auto &entry : fs::directory_iterator(outputDir)) {
        if (entry.path().extension() == ".txt" && entry.path().filename().string().find("block_times_") != std::string::npos) {
            std::string inputFile = entry.path().string();
            std::string peerID = entry.path().stem().string().substr(12); // Extract peer number

            std::string outputDotDir = "output/dot";
            std::string outputGraphDir = "output/graphs";
            
            // Ensure output directories exist
            fs::create_directories(outputDotDir);
            fs::create_directories(outputGraphDir);

            std::string outputDotFile = outputDotDir + "/block_tree_" + peerID + ".dot";
            std::string outputPngFile = outputGraphDir + "/block_tree_" + peerID + ".png";

            generateGraphvizDotFile(inputFile, outputDotFile, outputPngFile);
        }
    }

    return 0;
}
