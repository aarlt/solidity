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
/** @file SoltestExecutor.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTEXECUTOR_H
#define SOLIDITY_SOLTESTEXECUTOR_H

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <string>
#include <cstdint>

namespace dev
{

namespace soltest
{

class SoltestExecutor : private dev::solidity::ASTConstVisitor
{
public:
	SoltestExecutor(dev::solidity::SourceUnit const &sourceUnit,
					std::string const &contract,
					std::string const &filename,
					uint32_t line);

	bool execute(std::string const &testcase, std::string &errors);

private:
	dev::solidity::SourceUnit const &m_sourceUnit;
	std::string m_contract;
	std::string m_filename;
	uint32_t m_line;

	std::string m_errors;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTEXECUTOR_H
