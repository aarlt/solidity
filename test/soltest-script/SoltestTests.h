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
/** @file SoltestTests.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTTESTS_H
#define SOLIDITY_SOLTESTTESTS_H

#include <memory>
#include <map>
#include <unordered_map>
#include <string>

namespace dev
{

namespace soltest
{

class SoltestTests
{
public:
	typedef typename std::shared_ptr<SoltestTests> Ptr;
	explicit SoltestTests(std::string const &source, std::string const &file);

	std::vector<std::string> testcases();

	std::string file()
	{
		return this->m_file;
	}

	uint32_t line(std::string &name)
	{
		return m_lines[name];
	}

	std::string generateSolidity();

	std::string content(uint32_t indention, std::string const &section);

	std::string normalizeName(std::string const& name);

private:
	std::string m_content;
	std::string m_file;
	std::map<std::string, std::stringstream> m_tests;
	std::map<std::string, uint32_t> m_lines;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTTESTS_H
