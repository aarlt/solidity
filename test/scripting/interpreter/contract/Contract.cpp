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

namespace dev
{

namespace soltest
{

bool Contract::construct(StateTypes const &arguments)
{
	if (m_rpc != nullptr)
	{
		return remoteConstruct(arguments);
	}
	return true;
}

bool Contract::call(std::string const &methodName, StateTypes const &arguments, StateTypes &results)
{
	if (m_rpc != nullptr)
	{
		return remoteCall(methodName, arguments, results);
	}
	else
	{
		try
		{
			std::function<StateTypes(StateTypes)> contractMethod(m_methods[methodName]);
			StateTypes current = contractMethod(arguments);
			std::cout << methodName << " " << results.size() << " " << current.size() << std::endl;
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

bool Contract::remoteConstruct(StateTypes const &arguments)
{
	if (this->type.find("contract ") != std::string::npos && m_compilerStack != nullptr && m_rpc != nullptr)
	{
		std::string contractName(this->type.substr(9));
		eth::LinkerObject obj = m_compilerStack->object(contractName);
		BOOST_REQUIRE(obj.linkReferences.empty());
		if (arguments.empty())
			m_rpc->sendMessage(*this, obj.bytecode, true);

		// todo: multiple arguments

		return true;
	}
	return false;
}

bool Contract::remoteCall(std::string const &methodName, StateTypes const &arguments, StateTypes &results)
{
	(void) methodName;
	(void) arguments;
	(void) results;

	return true;
}

} // namespace soltest

} // namespace dev
