pragma solidity ^0.4.0;

contract mortal {
    address owner;
    function mortal() { owner = msg.sender; }
    function kill() { if (msg.sender == owner) selfdestruct(owner); }
}

/// @test { setup }
///       g = greeter("hello");
/// @test { greet }
///       assert(g.greeting == "hello");
/// @test { checks }
///       assert(g.f(1,2) == 3);
///       assert(g.f(2,3) == 5);
/// @test { teardown }
///       g.kill();
contract greeter is mortal {
    string greeting;
    
    function greeter(string _greeting) public {
        greeting = _greeting;
    }

    function f(uint a, uint b) public view returns (uint) {
        return a + b;
    }
    
    function greet() constant returns (string) {
        return greeting;
    }
}
