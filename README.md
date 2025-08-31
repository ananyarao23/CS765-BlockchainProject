# CS765 Blockchain Project

**Authors:** Sharvanee Sonawane (22B0943), Ananya Rao (22B0980), Deeksha Dhiwakar (22B0988)

This repository contains implementations for CS765 (Blockchain and Cryptocurrency) course assignments, featuring blockchain simulations, network protocols, and decentralized exchange implementations.

## Project Structure

### Assignment 1 (a1/) - Blockchain Simulation
A discrete event simulator for blockchain networks implementing basic peer-to-peer functionality.

**Key Features:**
- Block mining and propagation simulation
- Transaction generation and broadcasting
- Longest chain consensus protocol
- Peer categorization (slow/fast, low/high CPU)
- Hash power distribution modeling
- Network latency simulation

**Main Files:**
- `simulator.cpp/h` - Core simulation engine and peer management
- `blocks.cpp` - Block mining, validation, and tree management
- `transactions.cpp` - Transaction generation and propagation
- `helper.cpp/h` - Utility functions for graph generation and hash computation
- `visualiser.cpp` - Block tree visualization and analysis
- `plot_results.py` - Python script for plotting simulation results

### Assignment 2 (a2/) - Advanced Blockchain Network
Extended blockchain simulator with malicious peer support and eclipse attack implementation.

**Key Features:**
- Honest and malicious peer types
- Eclipse attack simulation
- Overlay network for malicious coordination
- Hash-first block propagation protocol
- Selfish mining strategies
- Private chain release mechanisms

**Main Files:**
- `simulator.cpp/h` - Enhanced simulation framework
- `network.cpp/h` - Dual network implementation (honest + overlay)
- `structure.cpp/h` - Data structures for blocks, peers, and tree nodes
- `blocks.cpp` - Advanced block handling with hash propagation
- Visualization outputs in `output/` directory

### Assignment 3 (a3/) - Decentralized Exchange (DEX)
Smart contract implementation of a decentralized exchange with arbitrage capabilities.

**Key Features:**
- ERC20 token contracts with permit functionality
- Constant product market maker (CPMM) DEX
- Liquidity provision and LP token management
- Arbitrage bot implementation
- Fee tracking and slippage analysis

**Main Files:**
- `DEX.sol` - Core decentralized exchange contract
- `Token.sol` - ERC20 token implementations (TokenA, TokenB)
- `LPToken.sol` - Liquidity provider token contract
- `arbitrage.sol` - Arbitrage opportunity detection and execution
- `simulate_DEX.js` - DEX functionality testing
- `simulate_arbitrage.js` - Arbitrage scenario simulation
- `plot.py` - Visualization of DEX metrics

## Build and Run Instructions

### Assignment 1 & 2
```bash
cd a1/  # or a2/
make clean
make
./sim <parameters>
```

**Simulation Parameters:**
- Number of peers
- Simulation time
- Transaction generation rate
- Block generation rate
- Network configuration

### Assignment 3
1. Deploy smart contracts using Remix IDE or Hardhat
2. Update contract addresses in simulation scripts
3. Run simulations:
```bash
node simulate_DEX.js
node simulate_arbitrage.js
python3 plot.py
```

## Key Concepts Implemented

### Blockchain Fundamentals
- **Consensus Protocol**: Longest chain rule implementation
- **Mining**: Proof-of-work simulation with exponential inter-arrival times
- **Transaction Pool**: Mempool management and validation
- **Block Validation**: Balance checking and transaction verification

### Network Simulation
- **P2P Network**: Random graph generation with degree constraints
- **Latency Modeling**: Realistic network delay simulation
- **Message Propagation**: Transaction and block broadcasting
- **Network Heterogeneity**: Slow/fast peers, varying CPU power

### Advanced Attacks
- **Eclipse Attack**: Isolating honest nodes from the network
- **Selfish Mining**: Strategic block withholding and release
- **51% Attack**: Majority hash power exploitation

### DeFi Protocols
- **Automated Market Maker**: Constant product formula (x * y = k)
- **Liquidity Provision**: Adding/removing liquidity with LP tokens
- **Arbitrage**: Cross-DEX price difference exploitation
- **Slippage**: Price impact calculation for large trades

## Visualization and Analysis

The project includes comprehensive visualization tools:
- Block tree generation (DOT format)
- Network topology graphs
- Mining statistics and fork analysis
- DEX metrics plotting (TVL, fees, prices)
- Arbitrage opportunity tracking

## Output Files

- `simulation_results.csv` - Peer statistics and blockchain metrics
- `output/graphs/` - Block tree visualizations
- `output/tree/` - Timeline data for each peer
- DEX simulation logs with trading metrics

## Dependencies

- **C++**: Standard library, OpenSSL for hashing
- **Python**: matplotlib, pandas (for plotting)
- **JavaScript**: Node.js (for smart contract simulations)
- **Solidity**: Smart contract compilation

## Academic Context

This project demonstrates practical implementations of:
- Blockchain consensus mechanisms
- Network security vulnerabilities
- Cryptocurrency economics
- Decentralized finance protocols
- Peer-to-peer network design

Each assignment builds upon the previous work, creating a comprehensive exploration of blockchain technology from basic simulation to advanced DeFi applications.