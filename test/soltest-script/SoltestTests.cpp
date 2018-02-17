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
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com
 * @date 2018
 */

#include "SoltestTests.h"

#include <sstream>
#include <iostream>

#include <boost/algorithm/string.hpp>

namespace dev
{

namespace soltest
{

SoltestTests::SoltestTests(std::string const &source, std::string const &file) : m_content(source), m_file(file)
{
	std::stringstream input(m_content);
	std::stringstream content;
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
				std::string section_nice(section.substr(1, section.length() - 2));
				m_tests[section_nice] = content.str();
				m_lines[section_nice] = count;
			}
			content.str("");
		}
		else
		{
			content << line;
			if (line.find("/*_soltest_line:") == std::string::npos)
			{
				content << " /*" << "_soltest_line:" << count << "*/";
			}
			content << std::endl;
		}
		++count;
	}
	if (section.length() > 2)
	{
		std::string section_nice(section.substr(1, section.length() - 2));
		m_tests[section_nice] = content.str();
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

} // namespace soltest

} // namespace dev