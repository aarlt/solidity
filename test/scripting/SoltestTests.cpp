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
/** @file SoltestTests.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "SoltestTests.h"

#include <sstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace dev
{

namespace soltest
{

SoltestTests::SoltestTests(std::string const &source, std::set<std::string> const &imports,
						   std::string const &file) : m_content(source), m_imports(imports), m_file(file)
{
	std::stringstream input(m_content);
	std::string section;
	uint32_t count(1);
	for (std::string line; getline(input, line);)
	{
		std::string trimmed(line);
		boost::trim(trimmed);
		if (boost::starts_with(trimmed, "{") && boost::ends_with(trimmed, "}"))
		{
			section = trimmed;
			if (section.length() > 2)
			{
				section = (section.substr(1, section.length() - 2));
				m_lines[section] = count;
			}
		}
		else
		{
			m_tests[section] << line;
			if (line.find("//_soltest_line:") == std::string::npos)
			{
				m_tests[section] << " //" << "_soltest_line:" << count;
			}
			m_tests[section] << std::endl;
		}
		++count;
	}
	if (section.length() > 2)
	{
		std::string section_nice(section.substr(1, section.length() - 2));
	}
}

std::vector<std::string> SoltestTests::testcases()
{
	std::vector<std::string> result;
	for (auto &test : m_tests)
	{
		if (test.first != "setup" && test.first != "teardown")
		{
			result.emplace_back(test.first);
		}
	}
	return result;
}

std::string SoltestTests::generateSolidity()
{
	std::stringstream result;
	std::string contractName("EndToEnd" + boost::filesystem::basename(m_file));
	result << "// " << m_file << std::endl;
	result << "pragma solidity ^0.4.0;" << std::endl;
	for (auto &import : m_imports)
	{
		result << "import \"" << import << "\";" << std::endl;
	}
	result << "import \"Soltest.sol\";" << std::endl;
	result << "contract " << contractName << " {" << std::endl;
	result << "    Soltest soltest;" << std::endl;
	for (auto &testcase : testcases())
	{
		result << "    function " << NormalizeName(testcase) << "() public {" << std::endl;
		result << content(8, "setup") << std::endl;
		result << content(8, testcase) << std::endl;
		result << content(8, "teardown");
		result << "    }" << std::endl;
	}
	result << "}" << std::endl;
	return result.str();
}

std::string SoltestTests::content(uint32_t indention, std::string const &section)
{
	std::stringstream result;
	std::stringstream input(m_tests[section].str());
	for (std::string line; getline(input, line);)
	{
		boost::trim(line);
		if (!line.empty())
		{
			for (uint32_t i = 0; i < indention; ++i)
			{
				result << " ";
			}
			result << line << std::endl;
		}
	}
	return result.str();
}

std::string SoltestTests::NormalizeName(std::string const &name)
{
	std::string result(name);
	std::replace_if(result.begin(), result.end(), ::ispunct, '_');
	std::replace_if(result.begin(), result.end(), ::isspace, '_');
	return result;
}

std::string SoltestTests::SoltestContract()
{
	// todo: generate the following contract code from information defined in test/scripting/interpreter/contract/SoltestContract.cpp
	char const *sourceCode = R"(
		pragma solidity ^0.4.0;
		contract Soltest {
			function setAccount(string account) public pure { }
			function setChainParams(uint a, uint b) public pure returns(uint d) { return a; }
			function mineBlocks(uint a) public pure returns(uint d) { return a; }
			function modifyTimestamp(uint a) public pure returns(uint d) { return a; }
			function addBlock(uint a) public pure returns(uint d) { return a; }
			function rewindToBlock(uint a) public pure returns(uint d) { return a; }
		}
	)";
	return sourceCode;
}

} // namespace soltest

} // namespace dev