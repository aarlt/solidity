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
/** @file SoltestClass.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "SoltestClass.h"
#include "SoltestState.h"

#include <boost/algorithm/string.hpp>

namespace dev
{

namespace soltest
{

std::vector<dev::soltest::StateType> SoltestClass::call(dev::soltest::MemberAccess const &member,
														std::vector<dev::soltest::AST_Type> const &arguments)
{
	std::vector<dev::soltest::StateType> args;
	std::vector<std::string> argumentTypes;
	std::vector<dev::soltest::StateType> returns;
	std::vector<std::string> returnTypes;
	std::string argumentTypesString(
		member.type.substr(member.type.find('(') + 1, member.type.find(')') - member.type.find('(') - 1)
	);
	boost::split(argumentTypes, argumentTypesString, boost::is_any_of(","));
	size_t argIdx = 0;
	for (auto &argumentType : argumentTypes)
	{
		StateType proto(CreateStateType(argumentType));
		if (arguments[argIdx].type() == typeid(dev::soltest::Literal))
		{
			soltest::Literal literal(boost::get<soltest::Literal>(arguments[argIdx]));
			args.push_back(LexicalCast(proto, literal.value));
		}
		++argIdx;
	}
	std::string returnTypesString(
		member.type.substr(member.type.rfind('(') + 1, member.type.rfind(')') - member.type.rfind('(') - 1)
	);
	boost::split(returnTypes, returnTypesString, boost::is_any_of(","));
	for (auto &returnType : returnTypes)
		returns.push_back(CreateStateType(returnType));

	std::cout << member.asString() << "( " << std::endl;
	for (auto &arg : args)
	{
		std::cout << "    " << " " << TypeAsString(arg) << " (" << ValueAsString(arg) << ")" << std::endl;
	}
	std::cout << ") -> (" << std::endl;
	if (member.member == "setChainParams")
	{
		this->setChainParams(args, returns);
	}
	for (auto &arg : returns)
	{
		std::cout << "    " << " " << TypeAsString(arg) << " (" << ValueAsString(arg) << ")" << std::endl;
	}
	std::cout << ")" << std::endl;

	return returns;
}

bool SoltestClass::setChainParams(std::vector<dev::soltest::StateType> const &arguments,
								  std::vector<dev::soltest::StateType> &returns)
{
	returns[0] = setChainParams(boost::get<u256>(arguments[0]), boost::get<u256>(arguments[1]));
	return true;
}

u256 SoltestClass::setChainParams(u256 a, u256 b)
{
	return a + b;
}

} // namespace soltest

} // namespace dev