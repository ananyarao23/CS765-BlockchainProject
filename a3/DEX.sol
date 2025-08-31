    // SPDX-License-Identifier: MIT
    pragma solidity ^0.8.20;

    import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
    import "./LPToken.sol";
    import "@openzeppelin/contracts/utils/math/Math.sol";

    contract DEX {
        IERC20 public tokenA;
        IERC20 public tokenB;
        LPToken public lpToken;

        uint256 public reserveA;
        uint256 public reserveB;

        uint256 public constant FEE_NUM = 997; // 0.3% fee
        uint256 public constant FEE_DEN = 1000;

        // --- Fee tracking per LP ---
        mapping(address => uint256) public feeDebtA;
        mapping(address => uint256) public feeDebtB;

        uint256 public accFeePerLPTokenA;
        uint256 public accFeePerLPTokenB;

        constructor(address _tokenA, address _tokenB) {
            tokenA = IERC20(_tokenA);
            tokenB = IERC20(_tokenB);
            lpToken = new LPToken("LP Token", "LPT");
            }

        function _updateReserves() internal {
            reserveA = tokenA.balanceOf(address(this));
            reserveB = tokenB.balanceOf(address(this));
        }


        function getReserves() external view returns (uint256, uint256) {
            return (reserveA, reserveB);
        }

        function spotPrice() external view returns (uint256, uint256) {
            return (reserveA, reserveB);
        }

        function addLiquidity(uint256 amountA, uint256 amountB) external {
            require(tokenA.transferFrom(msg.sender, address(this), amountA), "A transfer failed");
            require(tokenB.transferFrom(msg.sender, address(this), amountB), "B transfer failed");

            uint256 liquidity;
            if (lpToken.totalSupply() == 0) {
                liquidity = Math.sqrt(Math.mulDiv(amountA, amountB, 1));
            } 
            else {
                liquidity = (amountA * lpToken.totalSupply()) / reserveA;
            }
            // feeDebtA/B should ne the fee corresponding to the new token addition for that much accumulation
            feeDebtB[msg.sender] = ((lpToken.balanceOf(msg.sender)) * accFeePerLPTokenB) / 1e18;
            feeDebtA[msg.sender] = ((lpToken.balanceOf(msg.sender)) * accFeePerLPTokenA) / 1e18;
            lpToken.mint(msg.sender, liquidity);


            
            _updateReserves();
        }

        function removeLiquidity(uint256 lpAmount) external {

            uint256 userBalance = lpToken.balanceOf(msg.sender);

            uint256 pendingA = (userBalance * accFeePerLPTokenA) / 1e18 - feeDebtA[msg.sender];
            uint256 pendingB = (userBalance * accFeePerLPTokenB) / 1e18 - feeDebtB[msg.sender];

            uint256 totalSupply = lpToken.totalSupply();
            require(totalSupply > 0, "No liquidity");
            require(lpToken.balanceOf(msg.sender) >= lpAmount, "Insufficient LP balance");

            uint256 amountA = (lpAmount * reserveA) / totalSupply;
            uint256 amountB = (lpAmount * reserveB) / totalSupply;

            lpToken.burn(msg.sender, lpAmount);

            require(tokenA.transfer(msg.sender, amountA + pendingA), "A transfer failed");
            require(tokenB.transfer(msg.sender, amountB + pendingB), "B transfer failed");

            uint256 remainingLP = lpToken.balanceOf(msg.sender);
            
            if (remainingLP > 0) {
                feeDebtA[msg.sender] = (remainingLP * accFeePerLPTokenA) / 1e18;
                feeDebtB[msg.sender] = (remainingLP * accFeePerLPTokenB) / 1e18;
            } else {
                feeDebtA[msg.sender] = 0;
                feeDebtB[msg.sender] = 0;
            }
            _updateReserves();
        }

        function swap(address inputToken, uint256 inputAmount) external {
            require(inputToken == address(tokenA) || inputToken == address(tokenB), "Invalid token");
            require(inputAmount > 0, "Input amount must be > 0");

            bool isAtoB = inputToken == address(tokenA);
            IERC20 fromToken = isAtoB ? tokenA : tokenB;
            IERC20 toToken = isAtoB ? tokenB : tokenA;

            // Apply fee
            uint256 feeAmount = inputAmount * (FEE_DEN - FEE_NUM) / FEE_DEN;
            uint256 inputAmountWithFee = inputAmount - feeAmount;

            uint256 inputReserve = isAtoB ? reserveA : reserveB;
            uint256 outputReserve = isAtoB ? reserveB : reserveA;

            uint256 outputAmount = outputReserve - (inputReserve * outputReserve) / (inputReserve + inputAmountWithFee);

            require(fromToken.transferFrom(msg.sender, address(this), inputAmount), "Transfer failed");
            require(toToken.transfer(msg.sender, outputAmount), "Swap transfer failed");
            
            uint256 totalLPTokens = lpToken.totalSupply();
            if (totalLPTokens > 0) {
                if (isAtoB) {
                    accFeePerLPTokenA += (feeAmount * 1e18) / totalLPTokens;
                } else {
                    accFeePerLPTokenB += (feeAmount * 1e18) / totalLPTokens;
                }
            }



            _updateReserves();
        }
        
    }
