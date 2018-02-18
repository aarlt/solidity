/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file TestCaseGenerator.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com
 * @date 2018
 */

#include "TestCaseGenerator.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <iostream>
#include <memory>

namespace dev
{

namespace soltest
{

TestCaseGenerator::TestCaseGenerator(boost::unit_test::test_suite &_testSuite,
									 dev::solidity::CompilerStack &compilerStack,
									 std::set<std::string> const &_contracts)
	: m_testSuite(_testSuite), m_compilerStack(compilerStack), m_constracts(_contracts)
{
	for (auto &contract : m_constracts)
	{
		std::vector<std::string> components;
		boost::split(components, contract, boost::is_any_of(":"));
		if (components.size() == 2)
		{
			boost::filesystem::path path(components[0]);
			std::string soltest(
				path.parent_path().string() + boost::filesystem::path::separator + components[1] + ".soltest"
			);
			if (boost::filesystem::exists(soltest))
			{
				std::ifstream file(soltest);
				std::stringstream content;
				content << file.rdbuf();
				addContractTests(soltest, content.str());
			}
		}
	}
}

void TestCaseGenerator::registerTestCases()
{
	static std::vector<std::shared_ptr<std::string>> strings;
	for (auto &contract : m_contractTests)
	{
		for (auto &testcase : contract.second->testcases())
		{
			// tests are executed asynchronously, we need a valid reference to the dynamically created string,
			// where the c-string pointer need to be valid for a while.
			std::shared_ptr<std::string> filename(new std::string(contract.second->file()));
			strings.emplace_back(filename);

			auto testRunner = std::bind(&TestCaseGenerator::test,
										this,
										contract.first,
										testcase,
										filename->c_str(),
										contract.second->line(testcase));

			m_testSuite.add(
				boost::unit_test::make_test_case(
					boost::function<void()>(testRunner),
					boost::filesystem::basename(contract.first) + " " + testcase,
					filename->c_str(),
					contract.second->line(testcase))
			);
		}
	}
}

void TestCaseGenerator::addContractTests(std::string const &contract, const std::string &tests)
{
	m_contractTests[contract] =
		std::shared_ptr<dev::soltest::SoltestTests>(new dev::soltest::SoltestTests(tests, contract));
}

void TestCaseGenerator::test(std::string const &contract,
							 std::string const &testcase,
							 const char *filename,
							 uint32_t line)
{
	(void) contract;
	(void) testcase;
	(void) filename;
	(void) line;
	dev::solidity::SourceUnit const *sourceUnit = nullptr;
	try
	{
		sourceUnit = &m_compilerStack.ast("EndToEnd" + boost::filesystem::basename(contract));
	}
	catch (...)
	{
		sourceUnit = nullptr;
	}
	BOOST_REQUIRE(sourceUnit != nullptr);
	if (sourceUnit == nullptr)
	{
		std::cout << boost::filesystem::basename(contract) << " @ " << testcase << " [" << filename << ":" << line
				  << "]"
				  << std::endl;
	}
	BOOST_CHECK(true);
}

std::vector<dev::soltest::SoltestTests::Ptr> TestCaseGenerator::soltests()
{
	std::vector<SoltestTests::Ptr> result;
	for (auto &soltest : m_contractTests)
	{
		result.emplace_back(soltest.second);
	}
	return result;
}

} // namespace soltest

} // namespace dev
