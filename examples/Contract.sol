// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.5.0 ;
contract RyanCoin {
    mapping (address => uint) public balances;
    uint constant public PRICE = 2000000000000000; // 2 mETH as PRICE

    constructor() public {
    }

    function buyCoin() public payable{     // pay ETH to create coin
        require(msg.value > 0);
        balances[msg.sender] += (msg.value / PRICE);
    }

    function getBalance(
    ) public view returns(uint){
        return balances[msg.sender];
    }
}
