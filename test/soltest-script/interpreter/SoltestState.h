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

#ifndef SOLIDITY_SOLTESTSTATE_H
#define SOLIDITY_SOLTESTSTATE_H

#include "StateType.h"
#include <libsolidity/ast/AST.h>
#include <test/soltest-script/interpreter/SoltestStack.h>

#include <map>
#include <boost/algorithm/string.hpp>

namespace dev
{

namespace soltest
{

std::string TypeAsString(StateType const &type);

std::string ValueAsString(StateType const &type);

std::string RawValueAsString(StateType const &type);

class State : public std::map<std::string, StateType>
{
public:
	void set(std::string const &name, StateType const &type);

	void set(std::string const &name, AST_Type const &type);

	StateType &get(std::string const &name);

	void print();
};

StateType CreateStateType(std::string const &_typename);

StateType LexicalCast(StateType const &prototype, std::string const &_string);

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTSTATE_H
