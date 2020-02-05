//
// Created by Alexander Arlt on 05.02.20.
//
#pragma once

#include <test/Common.h>

#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/OptimiserSettings.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/Keccak256.h>

#include <functional>
#include <map>
#include <vector>

typedef std::map<std::string, std::function<void(void)>> TestSuite;

#define SolidityExecutionFramework solidity::test::ExtractorExecutionFramework

#define BOOST_FIXTURE_TEST_SUITE(A, B) \
	class End2EndExtractor : public B  \
	{                                  \
	  public:                          \
		End2EndExtractor()             \
		{
#define BOOST_AUTO_TEST_SUITE_END() \
	}                               \
                                    \
  public:                           \
	TestSuite &testsuite()          \
	{                               \
		return m_tests;             \
	}                               \
                                    \
  private:                          \
	TestSuite m_tests;              \
	}                               \
	;

#define BOOST_REQUIRE_EQUAL(A, B) if (A == B) {}
#define BOOST_REQUIRE(A) if (A) {}
#define BOOST_CHECK_EQUAL(A, B) if (A == B) {}
#define BOOST_CHECK(A) if (A) {}

#define BOOST_AUTO_TEST_CASE(X) m_tests[#X] = [this]()

#define ALSO_VIA_YUL(CODE) CODE

#define ABI_CHECK(result, expectation) do { \
	auto abiCheckResult = End2EndExtractor::compareAndCreateMessage((result), (expectation)); \
} while (0)
