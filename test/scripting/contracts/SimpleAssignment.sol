pragma solidity ^0.4.0;

contract SimpleAssignment {
    function SimpleAssignment(uint16 a, uint32 b) public {
    }

    function add(uint a, uint b) public pure returns (uint sum)  {
        return a + b;
    }

    function multi(uint a, uint b) public pure returns (uint sum, uint product)  {
        return (a + b, a * b);
    }
}
