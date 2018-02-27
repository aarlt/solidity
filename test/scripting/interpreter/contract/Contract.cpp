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
/** @file Contract.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "Contract.h"

#include <boost/algorithm/string.hpp>
#include <test/scripting/interpreter/SoltestState.h>

namespace dev
{

namespace soltest
{

bool Contract::construct(h160 _from, StateTypes const &arguments)
{
	if (m_remote)
	{
		return remoteConstruct(_from, arguments);
	}
	return true;
}

bool Contract::call(h160 _from, std::string const &methodName, StateTypes const &arguments, StateTypes &results)
{
	if (m_remote)
	{
		return remoteCall(_from, methodName, arguments, results);
	}
	else
	{
		try
		{
			std::function<StateTypes(StateTypes)> contractMethod(m_methods[methodName]);
			StateTypes current = contractMethod(arguments);
			if (results.size() != current.size())
			{
				return false;
			}
			for (size_t i = 0; i < results.size(); ++i)
			{
				if (results[i].type() != current[i].type())
				{
					return false;
				}
			}
			results = current;
			return true;
		}
		catch (const std::bad_function_call &e)
		{
			std::cout << e.what() << '\n';
		}
	}
	return false;
}

bool Contract::remoteConstruct(h160 _from, StateTypes const &_arguments)
{
	if (this->type.find("contract ") != std::string::npos && m_compilerStack != nullptr && m_rpc != nullptr)
	{
		std::string contractName(this->type.substr(9));
		eth::LinkerObject obj = m_compilerStack->object(contractName);
		BOOST_REQUIRE(obj.linkReferences.empty());
		bytes arguments;

		for (auto &arg : _arguments)
			arguments += ValueAsBytes(arg);

		m_rpc->sendMessage(*this, _from, obj.bytecode + arguments, true);

		return true;
	}
	return false;
}

bool Contract::remoteCall(h160 _from,
						  std::string const &_methodName,
						  StateTypes const &_arguments,
						  StateTypes &_results)
{
	bytes arguments;
	std::stringstream signature;
	signature << _methodName << "(";
	for (auto &arg : _arguments)
	{
		signature << TypeAsString(arg);
		if (&arg != &(*_arguments.rbegin()))
			signature << ",";
	}
	signature << ")";
	for (auto &arg : _arguments)
		arguments += ValueAsBytes(arg);
	bytes rawResult = m_rpc->callContractFunctionNoEncoding(*this, _from, signature.str(), arguments);
	for (auto &result : _results)
		result = LexicalCast(result, toHex(rawResult));
	return true;
}

} // namespace soltest

} // namespace dev
