# Homework 3: Building your own Decentralized Exchange

## Sharvanee Sonawane (22B0943), Ananya Rao (22B0980), Deeksha Dhiwakar (22B0988)

This repository contains smart contracts, scripts, and utilities for simulating a decentralized exchange (DEX) and arbitrage opportunities. Below is a brief description of each file in the folder:

## Files

### Smart Contracts
1. **[arbitrage.sol](arbitrage.sol)**  
   Implements an arbitrage contract that identifies and executes profitable arbitrage opportunities between two DEXes. It calculates potential profits and performs token swaps to exploit price differences.

2. **[DEX.sol](DEX.sol)**  
   Implements a decentralized exchange (DEX) with functionalities for adding/removing liquidity, swapping tokens, and tracking fees. It uses a constant product formula for token pricing.

3. **[LPToken.sol](LPToken.sol)**  
   Implements an ERC20-compliant liquidity provider (LP) token. This token is minted and burned by the DEX to represent liquidity shares.

4. **[Token.sol](Token.sol)**  
   Contains two ERC20 token contracts (`TokenA` and `TokenB`) with permit functionality. These tokens are used for trading and liquidity provision in the DEX.

### Simulation Scripts
5. **[simulate_dex.js](simulate_dex.js)**  
   Simulates the behavior of a DEX, including adding/removing liquidity and token swaps. It tracks metrics such as fees, slippage, and token distribution over time.

6. **[simulate_arbitrage.js](simulate_arbitrage.js)**  
   Simulates arbitrage opportunities between two DEXes. It tests both profitable and non-profitable scenarios to validate the arbitrage contract's behavior.

### Utilities
7. **[plot.py](plot.py)**  
   A Python script for visualizing metrics generated during the DEX simulation. It creates plots for fees, TVL, spot prices, slippage, and other key metrics.

## Usage
- Deploy the smart contracts using Remix or a similar tool.
- Fill in the addresses of the deployed contracts in the simulation scripts.
- Use the simulation scripts to test the DEX and arbitrage functionalities.
- Save the output in a .txt file and run `plot.py` on it to visualize the results of the simulations.