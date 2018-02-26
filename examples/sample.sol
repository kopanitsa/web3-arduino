pragma solidity ^0.4.18;

contract Sample {
    uint data;

    function set(uint d) public{
        data = d;
    }

    function get() public constant returns (uint retVal) {
        return data;
    }
}

