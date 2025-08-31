// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Permit.sol";

contract TokenA is ERC20, ERC20Permit {
    constructor(uint256 initialSupply) ERC20("TokenA", "TKA") ERC20Permit("TokenA") 
    {
        _mint(msg.sender, initialSupply);
    }
}

contract TokenB is ERC20, ERC20Permit {
    constructor(uint256 initialSupply) ERC20("TokenB", "TKB") ERC20Permit("TokenB") 
    {
        _mint(msg.sender, initialSupply);
    }
}