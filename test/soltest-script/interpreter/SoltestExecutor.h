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

#include <libsolidity/ast/ASTVisitor.h>
#include <test/soltest-script/interpreter/SoltestStack.h>
#include <test/soltest-script/interpreter/SoltestState.h>

#include <string>
#include <cstdint>

#include <boost/variant.hpp>

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
	void print(dev::solidity::ASTNode const &node);

	void endVisit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement) override;

	void endVisit(dev::solidity::VariableDeclaration const &_variableDeclaration) override;

	void endVisit(dev::solidity::Literal const &_literal) override;

	void endVisit(dev::solidity::Assignment const &_assignment) override;

	void endVisit(dev::solidity::BinaryOperation const &_binaryOperation) override;

	void endVisit(dev::solidity::Identifier const &_identifier) override;

	void endVisit(dev::solidity::TupleExpression const &_tuple) override;

	void endVisit(dev::solidity::UnaryOperation const &_unaryOperation) override;

	void endVisit(dev::solidity::FunctionCall const &_functionCall) override;

	void endVisit(dev::solidity::NewExpression const &_newExpression) override;

	void endVisit(dev::solidity::MemberAccess const &_memberAccess) override;

	void endVisit(dev::solidity::IndexAccess const &_indexAccess) override;

	dev::solidity::SourceUnit const &m_sourceUnit;
	std::string m_contract;
	std::string m_filename;
	uint32_t m_line;

	std::string m_errors;
	dev::soltest::Stack m_stack;
	dev::soltest::State m_state;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTEXECUTOR_H
