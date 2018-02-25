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
#include <test/scripting/interpreter/rpc/SoltestSession.h>

#include <libsolidity/interface/CompilerStack.h>

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
	explicit Contract(std::string const &_type,
					  dev::soltest::SoltestSession *_rpc = nullptr,
					  dev::solidity::CompilerStack *_compilerStack = nullptr)
		: type(_type), m_address(0), m_account(0), m_rpc(_rpc), m_compilerStack(_compilerStack)
	{
	}

	void setAccount(h160 &_account)
	{
		m_account = _account;
	}

	h160 account() const
	{
		return m_account;
	}

	void setAddress(h160 &_address)
	{
		m_address = _address;
	}

	h160 address() const
	{
		return m_address;
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

	std::string asString() const
	{
		std::stringstream stream;
		stream << "type = '" << type << "'";
		if (m_rpc != nullptr)
		{
			stream << " @ 0x" << m_address;
		}
		return stream.str();
	}

	bool construct(StateTypes const &arguments);

	bool call(std::string const &methodName, StateTypes const &arguments, StateTypes &results);

	std::string type;

private:
	bool remoteConstruct(StateTypes const &arguments);

	bool remoteCall(std::string const &methodName, StateTypes const &arguments, StateTypes &results);

	h160 m_address;
	h160 m_account;
	dev::soltest::SoltestSession *m_rpc;
	dev::solidity::CompilerStack *m_compilerStack;
	std::map<std::string, std::function<StateTypes(StateTypes)>> m_methods;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTCONTRACT_H
