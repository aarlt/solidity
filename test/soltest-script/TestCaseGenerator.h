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

#ifndef SOLIDITY_TESTCASEGENERATOR_H
#define SOLIDITY_TESTCASEGENERATOR_H

#include <libsolidity/interface/CompilerStack.h>

#include <boost/test/framework.hpp>
#include <boost/test/test_tools.hpp>

#include "SoltestTests.h"

namespace dev
{

namespace soltest
{

class TestCaseGenerator
{
public:
	explicit TestCaseGenerator(boost::unit_test::test_suite &_testSuite,
							   dev::solidity::CompilerStack &compilerStack,
							   std::set<std::string> const &_contracts = std::set<std::string>());

	void addContractTests(std::string const &_contract, std::string const &_tests);
	void registerTestCases();
	void test(std::string const &contract, std::string const &testcase, const char *filename, uint32_t line);

	std::vector<dev::soltest::SoltestTests::Ptr> soltests();

private:
	boost::unit_test::test_suite &m_testSuite;
	dev::solidity::CompilerStack &m_compilerStack;
	std::set<std::string> const &m_constracts;
	std::map<std::string, dev::soltest::SoltestTests::Ptr> m_contractTests;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_TESTCASEGENERATOR_H
