// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/token/ERC20/IERC20.sol";

interface IDex {
    function spotPrice() external view returns (uint256, uint256);
    function swap(address inputToken, uint256 inputAmount) external;
    function getReserves() external view returns (uint256, uint256);
}

contract Arbitrage {
    address public owner;
    uint256 public constant FEE_NUM = 997;
    uint256 public constant FEE_DEN = 1000;

    constructor() {
        owner = msg.sender;
    }

    // allows only the owner to execute the function
    modifier onlyOwner() {
        require(msg.sender == owner, "Only owner can execute");
        _;
    }

    function arbitrage(
        address dex1,
        address dex2,
        address tokenA,
        address tokenB,
        uint256 amountIn
    ) external onlyOwner {
        IERC20(tokenA).transferFrom(msg.sender, address(this), amountIn);

        uint256 minProfit = (amountIn * 5) / 10000; // 0.05% minimum profit threshold

        // checks all possible arbitrage opportunities and returns the first profitable one

        uint256 profitA1 = getProfit(1, dex1, dex2, amountIn);

        if (profitA1 > minProfit)
        {
            executeArbitrage(dex1, dex2, tokenA, tokenB, amountIn);
            return;
        }

        uint256 profitA2 = getProfit(1, dex2, dex1, amountIn);

        if (profitA2 > minProfit)
        {
            executeArbitrage(dex2, dex1, tokenA, tokenB, amountIn);
            return;
        }

        uint256 profitB1 = getProfit(2, dex1, dex2, amountIn);

        if (profitB1 > minProfit)
        {
            executeArbitrage(dex1, dex2, tokenB, tokenA, amountIn);
            return;
        }

        uint256 profitB2 = getProfit(2, dex2, dex1, amountIn);

        if (profitB2 > minProfit)
        {
            executeArbitrage(dex2, dex1, tokenB, tokenA, amountIn);
            return;
        }

        revert("No profitable arbitrage opportunity");
    }

    function getProfit(         // calculates the profit from an arbitrage opportunity
        uint256 token,
        address buyDex,
        address sellDex,
        uint256 amountIn
    ) public view returns (uint256) {
        (uint256 reserveA1, uint256 reserveB1) = IDex(buyDex).getReserves();
        (uint256 reserveA2, uint256 reserveB2) = IDex(sellDex).getReserves();

        uint256 amountInWithFee = amountIn * FEE_NUM / FEE_DEN;
        if (token == 1) // input is tokenA
        {
            uint256 amountOutTokenB = (reserveA1 * reserveB1) / (reserveA1 + amountInWithFee);
            uint256 amountBWithFee = amountOutTokenB * FEE_NUM / FEE_DEN;
            uint256 amountOutTokenA = (reserveB2 * reserveA2) / (reserveB2 + amountBWithFee);

            if (amountOutTokenA > amountIn) {
                return amountOutTokenA - amountIn;
            } else {
                return 0;
            }
        }
        else  // input is tokenB
        {
            uint256 amountOutTokenA = (reserveA1 * reserveB1) / (reserveB1 + amountInWithFee);
            uint256 amountBWithFee = amountOutTokenA * FEE_NUM / FEE_DEN;
            uint256 amountOutTokenB = (reserveB2 * reserveA2) / (reserveA2 + amountBWithFee);

            if (amountOutTokenB > amountIn) {
                return amountOutTokenB - amountIn;
            } else {
                return 0;
            }
        }
    }

    function executeArbitrage(      // actually executes the arbitrage opportunity by swapping tokens in the two DEXes
        address buyDex,
        address sellDex,
        address tokenA,
        address tokenB,
        uint256 amountIn
    ) internal {
        IERC20(tokenA).approve(buyDex, amountIn);
        IDex(buyDex).swap(tokenA, amountIn);

        uint256 tokenBBalance = IERC20(tokenB).balanceOf(address(this));
        IERC20(tokenB).approve(sellDex, tokenBBalance);
        IDex(sellDex).swap(tokenB, tokenBBalance);

        uint256 finalBalance = IERC20(tokenA).balanceOf(address(this));
        require(finalBalance > amountIn, "No profit made");

        IERC20(tokenA).transfer(owner, finalBalance);
    }
}
