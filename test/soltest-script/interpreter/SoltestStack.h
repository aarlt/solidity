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

#include <boost/variant.hpp>

namespace dev
{

namespace soltest
{

typedef boost::variant<
	Empty, TypeId,

	Literal, VariableDeclaration,

	bool,
	int8_t, int16_t, int32_t, int64_t, s256, // todo: int128_t
	uint8_t, uint16_t, uint32_t, uint64_t, u160, u256, // todo: uint128_t
	std::string,

	Address, Contract
> AST_Type;

std::string TypeAsString(AST_Type const &type);

std::string ValueAsString(AST_Type const &type);

class Stack : public std::vector<AST_Type>
{
public:
	void push(AST_Type element);

	AST_Type pop();

	Stack &operator<<(AST_Type const &_value);

	void print();
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTSTACK_H
