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
/** @file TestSuiteGenerator.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com
 * @date 2018
 */

#include "TestSuiteGenerator.h"

#include <libsolidity/interface/StandardCompiler.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <iostream>
#include <fstream>
#include <libsolidity/interface/SourceReferenceFormatter.h>

namespace dev
{

namespace soltest
{

TestSuiteGenerator::TestSuiteGenerator(boost::unit_test::master_test_suite_t &_masterTestSuite)
	: m_running(true), m_masterTestSuite(_masterTestSuite)
{
	m_masterTestSuite.p_name.value = "Contracts";
	m_contractsTestSuites = BOOST_TEST_SUITE("contracts");
	m_masterTestSuite.add(m_contractsTestSuites);
}

bool TestSuiteGenerator::parseCommandLineArguments(int argc, char **argv)
{
	for (auto i = 0; i < argc; i++)
	{
		std::string argument(argv[i]);
		if (argument == "--ipcpath")
		{
			m_options[argument] = argv[i + 1];
			++i;
		}
		else if (boost::starts_with(argument, "-"))
		{
			m_options[argument] = "yes";
		}
		else if (boost::filesystem::exists(argument))
		{
			if (boost::filesystem::extension(argument) == ".sol")
			{
				m_contracts.insert(argument);
				if (boost::filesystem::exists(argument + "test"))
				{
					m_tests.insert(argument + "test");
				}
			}
			else if (boost::filesystem::extension(argument) == ".soltest")
			{
				std::string contractFile(argument.substr(0, argument.length() - 4));
				m_tests.insert(argument);
				if (boost::filesystem::exists(contractFile))
				{
					m_contracts.insert(contractFile);
				}
			}
		}
	}

	if (preloadContracts(m_contracts))
	{
		for (auto &contract : m_compilerStack.contractNames())
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
					std::cout << soltest << std::endl;
					m_tests.insert(soltest);
				}
			}
		}
	}

	BOOST_TEST_MESSAGE("- options");
	for (auto &option : m_options)
	{
		BOOST_TEST_MESSAGE("    '" + option.first + "' = '" + option.second + "'");
	}
	BOOST_TEST_MESSAGE("- contracts");
	for (auto &contract : m_contracts)
	{
		BOOST_TEST_MESSAGE("    '" + contract + "'");
	}
	BOOST_TEST_MESSAGE("- tests");
	for (auto &test : m_tests)
	{
		BOOST_TEST_MESSAGE("    '" + test + "'");
	}

	static auto loadContracts = std::bind(&TestSuiteGenerator::loadContracts, this, m_contracts);
	m_contractsTestSuites->add(
		boost::unit_test::make_test_case(boost::function<void()>(loadContracts), "load contracts", __FILE__, __LINE__)
	);

	static auto parseContracts = std::bind(&TestSuiteGenerator::parseContracts, this);
	m_contractsTestSuites->add(
		boost::unit_test::make_test_case(boost::function<void()>(parseContracts),
										 "parsing contracts", __FILE__, __LINE__)
	);

	static auto analyzeContracts = std::bind(&TestSuiteGenerator::analyzeContracts, this);
	m_contractsTestSuites->add(
		boost::unit_test::make_test_case(boost::function<void()>(analyzeContracts),
										 "analyzing contracts", __FILE__, __LINE__)
	);

	static auto compileContracts = std::bind(&TestSuiteGenerator::compileContracts, this);
	m_contractsTestSuites->add(
		boost::unit_test::make_test_case(boost::function<void()>(compileContracts),
										 "compile contracts", __FILE__, __LINE__)
	);

	return true;
}

bool TestSuiteGenerator::preloadContracts(std::set<std::string> const &contracts)
{
	bool success(false);
	m_compilerStack.reset();
	for (auto &contract : contracts)
	{
		std::ifstream file(contract);
		std::stringstream content;
		content << file.rdbuf();
		success = !m_compilerStack.addSource(contract, content.str());
	}
	if (success)
		success = m_compilerStack.parseAndAnalyze();
	return success;
}

void TestSuiteGenerator::loadContracts(std::set<std::string> const &contracts)
{
	m_compilerStack.reset();
	m_scannerFromSourceName =
		[&](std::string const &_sourceName) -> solidity::Scanner const &
		{
			return m_compilerStack.scanner(_sourceName);
		};
	for (auto &contract : contracts)
	{
		BOOST_TEST_MESSAGE("loading contract '" + contract + "'");
		std::ifstream file(contract);
		std::stringstream content;
		content << file.rdbuf();
		bool success = !m_compilerStack.addSource(contract, content.str());
		m_running = m_running && success;
		BOOST_REQUIRE_MESSAGE(success, errors());
	}
}

void TestSuiteGenerator::parseContracts()
{
	BOOST_REQUIRE_MESSAGE(m_running, "Aborting.");
	bool success = m_compilerStack.parse();
	m_running = m_running && success;
	BOOST_REQUIRE_MESSAGE(success, errors());
}

void TestSuiteGenerator::analyzeContracts()
{
	BOOST_REQUIRE_MESSAGE(m_running, "Aborting.");
	bool success = m_compilerStack.analyze();
	m_running = m_running && success;
	BOOST_REQUIRE_MESSAGE(success, errors());
}

void TestSuiteGenerator::compileContracts()
{
	BOOST_REQUIRE_MESSAGE(m_running, "Aborting.");
	bool success = m_compilerStack.compile();
	m_running = m_running && success;
	BOOST_REQUIRE_MESSAGE(success, errors());
}

std::string TestSuiteGenerator::errors()
{
	std::stringstream result;
	for (auto const &error: m_compilerStack.errors())
	{
		auto const &err = dynamic_cast<dev::solidity::Error const &>(*error);
		std::string formattedMessage = dev::solidity::SourceReferenceFormatter::formatExceptionInformation(
			*error, err.typeName(), m_scannerFromSourceName
		);
		result << std::endl << formattedMessage;
	}
	return result.str();
}

} // namespace soltest

} // namespace dev
