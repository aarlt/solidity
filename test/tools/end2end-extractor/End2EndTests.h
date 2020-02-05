#pragma once

#include "End2EndExtractor.hpp"
#include "ExtractorExecutionFramework.h"

using namespace solidity;

#define END_TO_END_EXTRACTOR
#ifdef END_TO_END_EXTRACTOR
using namespace std;
using namespace std::placeholders;
using namespace solidity::util;
using namespace solidity::test;
using namespace solidity::frontend;
#include "test/libsolidity/SolidityEndToEndTest.cpp"
#else
BOOST_FIXTURE_TEST_SUITE(SolidityEndToEndTest, SolidityExecutionFramework)

BOOST_AUTO_TEST_CASE(exp_operator)
{
	char const *sourceCode = R"(
		contract test {
			function f(uint a) public returns(uint d) { return 2 ** a; }
		}
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange(
	    "f(uint256)", [](u256 const &a) -> u256 { return u256(1 << a.convert_to<int>()); }, 0, 16);
};

BOOST_AUTO_TEST_CASE(exp_zero)
{
	char const *sourceCode = R"(
		contract test {
			function f(uint a) public returns(uint d) { return a ** 0; }
		}
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange(
	    "f(uint256)", [](u256 const &) -> u256 { return u256(1); }, 0, 16);
};

BOOST_AUTO_TEST_CASE(recursive_calls)
{
	char const *sourceCode = R"(
		contract test {
			function f(uint n) public returns(uint nfac) {
				if (n <= 1) return 1;
				else return n * f(n - 1);
			}
		}
	)";
	compileAndRun(sourceCode);
};

BOOST_AUTO_TEST_SUITE_END()

#endif
