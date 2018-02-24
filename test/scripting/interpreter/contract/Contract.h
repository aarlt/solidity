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
/** @file Contract.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTCONTRACT_H
#define SOLIDITY_SOLTESTCONTRACT_H

#include <string>
#include <iostream>
#include <sstream>

#include <test/scripting/interpreter/SoltestAST.h>
#include <boost/variant.hpp>

namespace dev
{

namespace soltest
{

// forward declaration fun :D

class Contract;

using StateType = boost::variant<
	Empty,

	bool, /* double, // not  yet implemented in solidity */
	int8_t, int16_t, int32_t, int64_t, s256, // todo: int128_t
	uint8_t, uint16_t, uint32_t, uint64_t, u160, u256, // todo: uint128_t
	std::string,

	Address, Contract
>;

using StateTypes = std::vector<StateType>;

class Contract
{
public:
	explicit Contract(std::string const &_type) : type(_type)
	{
	}

	std::string asString() const
	{
		std::stringstream stream;
		stream << "type = '" << type << "'";
		return stream.str();
	}

	bool call(std::string const &methodName, StateTypes const &arguments, StateTypes &results)
	{
		std::cout << methodName << "(" << arguments.size() << " X " << results.size() << ")" << std::endl;
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
		return false;
	}

	template<typename TContract, typename TReturn, typename TArgument0>
	void registerContractMethod(std::string const &name,
								TContract &self,
								std::function<TReturn(TContract &, TArgument0)> function)
	{
		m_methods[name] = [&self, function](StateTypes args) -> StateTypes
		{
			TArgument0 argument0 = boost::get<TArgument0>(args[0]);
			StateTypes result;
			result.push_back(function(self, argument0));
			return result;
		};
	}

	template<typename TContract, typename TReturn, typename TArgument0, typename TArgument1>
	void registerContractMethod(std::string const &name, TContract &self,
								std::function<TReturn(TContract &, TArgument0, TArgument1)> function)
	{
		m_methods[name] = [&self, function](StateTypes args) -> StateTypes
		{
			TArgument0 argument0 = boost::get<TArgument0>(args[0]);
			TArgument1 argument1 = boost::get<TArgument1>(args[1]);
			StateTypes result;
			result.push_back(function(self, argument0, argument1));
			return result;
		};
	}
	std::string type;

private:
	std::map<std::string, std::function<StateTypes(StateTypes)>> m_methods;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTCONTRACT_H
