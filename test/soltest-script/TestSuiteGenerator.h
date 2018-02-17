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
/** @file TestSuiteGenerator.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com
 * @date 2018
 */

#ifndef SOLIDITY_TESTSUITEGENERATOR_HPP
#define SOLIDITY_TESTSUITEGENERATOR_HPP

#include <boost/test/framework.hpp>

#include <map>
#include <boost/test/test_tools.hpp>

#include <libdevcore/JSON.h>
#include <libsolidity/interface/CompilerStack.h>

namespace dev
{

namespace soltest
{

class TestSuiteGenerator
{
public:
	explicit TestSuiteGenerator(boost::unit_test::master_test_suite_t &_masterTestSuite);
	bool parseCommandLineArguments(int argc, char *argv[]);

	boost::unit_test::test_suite *test_suite() {
		return this->m_contractsTestSuites;
	}

private:
	bool preloadContracts(std::set<std::string> const& contracts);
	void loadContracts(std::set<std::string> const& contracts);
	void parseContracts();
	void analyzeContracts();
	void compileContracts();

	std::string errors();

	bool m_running;

	boost::unit_test::master_test_suite_t &m_masterTestSuite;
	boost::unit_test::test_suite *m_contractsTestSuites;

	std::map<std::string, std::string> m_options;
	std::set<std::string> m_contracts;
	std::set<std::string> m_tests;

	std::function<const solidity::Scanner &(const std::string &)> m_scannerFromSourceName;
	dev::solidity::CompilerStack m_compilerStack;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_TESTSUITEGENERATOR_HPP
