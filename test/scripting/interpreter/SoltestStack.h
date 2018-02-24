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
/** @file SoltestStack.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTSTACK_H
#define SOLIDITY_SOLTESTSTACK_H

#include "SoltestAST.h"
#include "StateType.h"

#include <boost/variant.hpp>

namespace dev
{

namespace soltest
{

typedef boost::variant<
	TypeId,

	Literal, VariableDeclaration, BinaryOperation, Identifier, MemberAccess, NewExpression,

	StateType
> AST_Type;

typedef std::vector<AST_Type> AST_Types;

std::string TypeAsString(AST_Type const &type);

std::string ValueAsString(AST_Type const &type);

Literal Evaluate(Literal const &left, std::string const &op, Literal const &right);

Literal Evaluate(StateType const &type);

class Stack : public std::vector<AST_Type>
{
public:
	void push(AST_Type element);

	AST_Type pop();

	AST_Type first();

	Stack &operator<<(AST_Type const &_value);

	void print();
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTSTACK_H
