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
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "TestSuiteGenerator.h"

#include "TestCaseGenerator.h"

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
	m_masterTestSuite.p_name.value = "End To End Tests";
	m_contractsTestSuite = BOOST_TEST_SUITE("EndToEnd");
	m_masterTestSuite.add(m_contractsTestSuite);
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
				m_contractFiles.insert(argument);
			}
			else if (boost::filesystem::extension(argument) == ".soltest")
			{
				std::string contractFile(argument.substr(0, argument.length() - 4));
				if (boost::filesystem::exists(contractFile))
				{
					m_contractFiles.insert(contractFile);
				}
			}
		}
	}

	if (preloadContracts(m_contractFiles))
	{
		for (auto &contract : m_compilerStack.contractNames())
		{
			m_contracts.insert(contract);
		}
	}

	BOOST_TEST_MESSAGE("- options");
	for (auto &option : m_options)
	{
		BOOST_TEST_MESSAGE("    '" + option.first + "' = '" + option.second + "'");
	}
	BOOST_TEST_MESSAGE("- contracts");
	for (auto &contract : m_contractFiles)
	{
		BOOST_TEST_MESSAGE("    '" + contract + "'");
	}

	m_compilerStack.reset();

	static TestCaseGenerator testCaseGenerator(*m_contractsTestSuite, m_compilerStack, m_contracts);

	m_compilerStack.addSource("Soltest.sol", SoltestTests::SoltestContract());

	for (auto &soltest : testCaseGenerator.soltests())
	{
		std::string contractName("EndToEnd" + boost::filesystem::basename(soltest->file()));
		std::string contractSource(soltest->generateSolidity());
		m_compilerStack.addSource(contractName, contractSource);
	}

	auto loadContracts = std::bind(&TestSuiteGenerator::loadContracts, this, m_contractFiles);
	m_contractsTestSuite->add(
		boost::unit_test::make_test_case(boost::function<void()>(loadContracts), "load contracts", __FILE__, __LINE__)
	);

	auto parseContracts = std::bind(&TestSuiteGenerator::parseContracts, this);
	m_contractsTestSuite->add(
		boost::unit_test::make_test_case(boost::function<void()>(parseContracts),
										 "parsing contracts", __FILE__, __LINE__)
	);

	auto analyzeContracts = std::bind(&TestSuiteGenerator::analyzeContracts, this);
	m_contractsTestSuite->add(
		boost::unit_test::make_test_case(boost::function<void()>(analyzeContracts),
										 "analyzing contracts", __FILE__, __LINE__)
	);

	auto compileContracts = std::bind(&TestSuiteGenerator::compileContracts, this);
	m_contractsTestSuite->add(
		boost::unit_test::make_test_case(boost::function<void()>(compileContracts),
										 "compile contracts", __FILE__, __LINE__)
	);

	testCaseGenerator.registerTestCases();

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
