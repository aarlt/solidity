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
	bool visit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement) override;

	void endVisit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement) override;

	bool visit(dev::solidity::VariableDeclaration const &_variableDeclaration) override;

	void endVisit(dev::solidity::VariableDeclaration const &_variableDeclaration) override;

	bool visit(dev::solidity::Literal const &_literal) override;

	bool visit(dev::solidity::Assignment const &_assignment) override;

	bool visit(dev::solidity::TupleExpression const &_tuple) override;

	bool visit(dev::solidity::UnaryOperation const &_unaryOperation) override;

	bool visit(dev::solidity::BinaryOperation const &_binaryOperation) override;

	bool visit(dev::solidity::FunctionCall const &_functionCall) override;

	bool visit(dev::solidity::NewExpression const &_newExpression) override;

	bool visit(dev::solidity::MemberAccess const &_memberAccess) override;

	bool visit(dev::solidity::IndexAccess const &_indexAccess) override;

	typedef boost::variant<bool,
						   int8_t, int16_t, int32_t, int64_t, s256, // todo: int128_t
						   uint8_t, uint16_t, uint32_t, uint64_t, u256, // todo: uint128_t
						   h160, std::string> StateType;
	typedef std::pair<std::string, StateType> TypedData;

	StateType lexical_cast(StateType const&_type, std::string const& _string);

	dev::solidity::SourceUnit const &m_sourceUnit;
	std::string m_contract;
	std::string m_filename;
	uint32_t m_line;

	std::string m_errors;


	std::map<std::string, StateType> m_state;
	std::map<std::string, std::string> m_contracts;

	std::vector<TypedData> m_stack;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTEXECUTOR_H
