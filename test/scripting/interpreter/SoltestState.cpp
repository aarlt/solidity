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

#include "SoltestState.h"
#include "SoltestStack.h"

#include "test/ExecutionFramework.h"

namespace dev
{

namespace soltest
{

std::string TypeAsString(StateType const &type)
{
	if (type.type() == typeid(bool))
		return "bool";
	else if (type.type() == typeid(int8_t))
		return "int8";
	else if (type.type() == typeid(int16_t))
		return "int16";
	else if (type.type() == typeid(int32_t))
		return "int32";
	else if (type.type() == typeid(int64_t))
		return "int64";
	else if (type.type() == typeid(s256))
		return "int256";
	else if (type.type() == typeid(uint8_t))
		return "uint8";
	else if (type.type() == typeid(uint16_t))
		return "uint16";
	else if (type.type() == typeid(uint32_t))
		return "uint32";
	else if (type.type() == typeid(uint64_t))
		return "uint64";
	else if (type.type() == typeid(u160))
		return "uint160";
	else if (type.type() == typeid(u256))
		return "uint256";
	else if (type.type() == typeid(std::string))
		return "string";

	else if (type.type() == typeid(Address))
		return "address";
	else if (type.type() == typeid(Contract))
		return "Contract";

	return "?";
}

std::string RawValueAsString(StateType const &type)
{
	std::stringstream result;
	if (type.type() == typeid(bool))
		result << boost::get<bool>(type);
	else if (type.type() == typeid(int8_t))
		result << boost::get<int8_t>(type);
	else if (type.type() == typeid(int16_t))
		result << boost::get<int16_t>(type);
	else if (type.type() == typeid(int32_t))
		result << boost::get<int32_t>(type);
	else if (type.type() == typeid(int64_t))
		result << boost::get<int64_t>(type);
	else if (type.type() == typeid(s256))
		result << boost::get<s256>(type);
	else if (type.type() == typeid(uint8_t))
		result << boost::get<uint8_t>(type);
	else if (type.type() == typeid(uint16_t))
		result << boost::get<uint16_t>(type);
	else if (type.type() == typeid(uint32_t))
		result << boost::get<uint32_t>(type);
	else if (type.type() == typeid(uint64_t))
		result << boost::get<uint64_t>(type);
	else if (type.type() == typeid(u160))
		result << boost::get<u160>(type);
	else if (type.type() == typeid(u256))
		result << boost::get<u256>(type);
	else if (type.type() == typeid(std::string))
		result << boost::get<std::string>(type);

	else if (type.type() == typeid(Address))
		result << boost::get<Address>(type).value();
	else if (type.type() == typeid(Contract))
		result << boost::get<Contract>(type).asString();

	return result.str();
}

bytes ValueAsBytes(StateType const &type)
{
	if (type.type() == typeid(bool))
		return dev::test::ExecutionFramework::encode(boost::get<bool>(type));
	else if (type.type() == typeid(int8_t))
		return dev::test::ExecutionFramework::encode(boost::get<int8_t>(type));
	else if (type.type() == typeid(int16_t))
		return dev::test::ExecutionFramework::encode(u256(boost::get<int16_t>(type)));
	else if (type.type() == typeid(int32_t))
		return dev::test::ExecutionFramework::encode(u256(boost::get<int32_t>(type)));
	else if (type.type() == typeid(int64_t))
		return dev::test::ExecutionFramework::encode(u256(boost::get<int64_t>(type)));
	else if (type.type() == typeid(s256))
		return dev::test::ExecutionFramework::encode(u256(boost::get<s256>(type)));
	else if (type.type() == typeid(uint8_t))
		return dev::test::ExecutionFramework::encode(u256(boost::get<uint8_t>(type)));
	else if (type.type() == typeid(uint16_t))
		return dev::test::ExecutionFramework::encode(u256(boost::get<uint16_t>(type)));
	else if (type.type() == typeid(uint32_t))
		return dev::test::ExecutionFramework::encode(u256(boost::get<uint32_t>(type)));
	else if (type.type() == typeid(uint64_t))
		return dev::test::ExecutionFramework::encode(u256(boost::get<uint64_t>(type)));
	else if (type.type() == typeid(u160))
		return dev::test::ExecutionFramework::encode(u256(boost::get<u160>(type)));
	else if (type.type() == typeid(u256))
		return dev::test::ExecutionFramework::encode(boost::get<u256>(type));
	else if (type.type() == typeid(std::string))
		return dev::test::ExecutionFramework::encode(boost::get<std::string>(type));
	else if (type.type() == typeid(Address))
		return dev::test::ExecutionFramework::encode(u256(boost::get<Address>(type).value()));
	else if (type.type() == typeid(Contract))
		return dev::test::ExecutionFramework::encode(u256("0x" + boost::get<Contract>(type).address().hex()));

	return bytes();
}

std::string ValueAsString(StateType const &type)
{
	std::stringstream result;
	if (type.type() == typeid(bool))
		result << boost::get<bool>(type);
	else if (type.type() == typeid(int8_t))
		result << boost::get<int8_t>(type);
	else if (type.type() == typeid(int16_t))
		result << boost::get<int16_t>(type) << std::hex << " = 0x" << boost::get<int16_t>(type);
	else if (type.type() == typeid(int32_t))
		result << boost::get<int32_t>(type) << std::hex << " = 0x" << boost::get<int32_t>(type);
	else if (type.type() == typeid(int64_t))
		result << boost::get<int64_t>(type) << std::hex << " = 0x" << boost::get<int64_t>(type);
	else if (type.type() == typeid(s256))
		result << boost::get<s256>(type) << std::hex << " = 0x" << boost::get<s256>(type);
	else if (type.type() == typeid(uint8_t))
		result << boost::get<uint8_t>(type) << std::hex << " = 0x" << boost::get<uint8_t>(type);
	else if (type.type() == typeid(uint16_t))
		result << boost::get<uint16_t>(type) << std::hex << " = 0x" << boost::get<uint16_t>(type);
	else if (type.type() == typeid(uint32_t))
		result << boost::get<uint32_t>(type) << std::hex << " = 0x" << boost::get<uint32_t>(type);
	else if (type.type() == typeid(uint64_t))
		result << boost::get<uint64_t>(type) << std::hex << " = 0x" << boost::get<uint64_t>(type);
	else if (type.type() == typeid(u160))
		result << boost::get<u160>(type) << std::hex << " = 0x" << boost::get<u160>(type);
	else if (type.type() == typeid(u256))
		result << boost::get<u256>(type) << std::hex << " = 0x" << boost::get<u256>(type);
	else if (type.type() == typeid(std::string))
		result << boost::get<std::string>(type);

	else if (type.type() == typeid(Address))
		result << boost::get<Address>(type).value() << std::hex << " = 0x" << boost::get<Address>(type).value();
	else if (type.type() == typeid(Contract))
		result << boost::get<Contract>(type).asString();

	return result.str();
}

StateType CreateStateType(std::string const &_typename)
{
	if (_typename == "bool")
		return bool();
	else if (_typename == "int8")
		return int8_t();
	else if (_typename == "int16")
		return int16_t();
	else if (_typename == "int32")
		return int32_t();
	else if (_typename == "int64")
		return int64_t();
	else if (_typename == "int256")
		return s256();
	else if (_typename == "uint8")
		return uint8_t();
	else if (_typename == "uint16")
		return uint16_t();
	else if (_typename == "uint32")
		return uint32_t();
	else if (_typename == "uint64")
		return uint64_t();
	else if (_typename == "uint256")
		return u256();
	else if (_typename == "address")
		return Address();
	else if (_typename == "string")
		return std::string();
	else if (boost::starts_with(_typename, "contract "))
		return Contract(_typename);

	return Empty();
}

StateType LexicalCast(StateType const &prototype, std::string const &_string)
{
	std::string string(_string);
	if (boost::starts_with(_string, "0x"))
	{
		u256 raw;
		std::stringstream ss;
		ss << std::hex << _string;
		ss >> raw;
		string = RawValueAsString(raw);
	}
	StateType result;
	try
	{
		if (_string.find('.') == std::string::npos)
		{
			if (prototype.type() == typeid(bool))
			{
				if (string == "true")
					result = boost::lexical_cast<bool>(1);
				else if (string == "false")
					result = boost::lexical_cast<bool>(0);
				else
					result = boost::lexical_cast<bool>(string);
			}
			else if (prototype.type() == typeid(int8_t))
				result = boost::lexical_cast<int8_t>(string);
			else if (prototype.type() == typeid(int16_t))
				result = boost::lexical_cast<int16_t>(string);
			else if (prototype.type() == typeid(int32_t))
				result = boost::lexical_cast<int32_t>(string);
			else if (prototype.type() == typeid(int64_t))
				result = boost::lexical_cast<int64_t>(string);
			else if (prototype.type() == typeid(s256))
				result = boost::lexical_cast<s256>(string);
			else if (prototype.type() == typeid(uint8_t))
				result = boost::lexical_cast<uint8_t>(string);
			else if (prototype.type() == typeid(uint16_t))
				result = boost::lexical_cast<uint16_t>(string);
			else if (prototype.type() == typeid(uint32_t))
				result = boost::lexical_cast<uint32_t>(string);
			else if (prototype.type() == typeid(uint64_t))
				result = boost::lexical_cast<uint64_t>(string);
			else if (prototype.type() == typeid(u160))
				result = boost::lexical_cast<u160>(string);
			else if (prototype.type() == typeid(u256))
				result = boost::lexical_cast<u256>(string);
			else if (prototype.type() == typeid(Address))
				result = Address(u160(_string));
			else if (prototype.type() == typeid(Contract))
				result = Contract(_string);
			else if (prototype.type() == typeid(std::string))
				result = _string;
		}
		else
		{
			// fix-point not yet implemented
			result = Empty();
		}
	}
	catch (...)
	{
		result = Empty();
	}
	return result;
}

StateType CreateStateType(AST_Type const &type)
{
	if (boost::get<StateType>(type).type() == typeid(Empty()))
		return boost::get<Empty>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(bool))
		return boost::get<bool>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(int8_t))
		return boost::get<int8_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(int16_t))
		return boost::get<int16_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(int32_t))
		return boost::get<int32_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(int64_t))
		return boost::get<int64_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(s256))
		return boost::get<s256>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(uint8_t))
		return boost::get<uint8_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(uint16_t))
		return boost::get<uint16_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(uint32_t))
		return boost::get<uint32_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(uint64_t))
		return boost::get<uint64_t>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(u160))
		return boost::get<u160>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(u256))
		return boost::get<u256>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(std::string))
		return boost::get<std::string>(boost::get<StateType>(type));

	else if (boost::get<StateType>(type).type() == typeid(Address))
		return boost::get<Address>(boost::get<StateType>(type));
	else if (boost::get<StateType>(type).type() == typeid(Contract))
		return boost::get<Contract>(boost::get<StateType>(type));

	return Empty();
}

StateTypes CreateArgumentStateTypesFromFunctionType(std::string const &_functionType,
													dev::soltest::AST_Types const &_untypedTuple,
													dev::soltest::State &state)
{
	StateTypes args;
	std::vector<std::string> argumentTypes;
	std::string argumentTypesString(
		_functionType.substr(_functionType.find('(') + 1, _functionType.find(')') - _functionType.find('(') - 1)
	);
	boost::split(argumentTypes, argumentTypesString, boost::is_any_of(","));
	size_t argIdx = 0;
	for (auto &argumentType : argumentTypes)
	{
		std::string argType(argumentType);
		if (argType == "string memory") {
			argType = "string";
		}
		if (argIdx < _untypedTuple.size())
		{
			StateType proto(CreateStateType(argType));
			if (_untypedTuple[argIdx].type() == typeid(dev::soltest::Literal))
			{
				soltest::Literal literal(boost::get<soltest::Literal>(_untypedTuple[argIdx]));
				args.push_back(LexicalCast(proto, literal.value));
			}
			else if (_untypedTuple[argIdx].type() == typeid(dev::soltest::Identifier))
			{
				soltest::Identifier identifier(boost::get<soltest::Identifier>(_untypedTuple[argIdx]));
				args.push_back(LexicalCast(CreateStateType(argType),
										   RawValueAsString(state[identifier.name])));
			}
			++argIdx;
		}
	}
	return args;
}

StateTypes CreateReturnStateTypesFromFunctionType(std::string const &_functionType)
{
	StateTypes result;
	// if function is not a void function
	if (_functionType.rfind('(') != _functionType.find('(') )
	{
		std::vector<std::string> returnTypes;
		std::string returnTypesString(
			_functionType.substr(_functionType.rfind('(') + 1, _functionType.rfind(')') - _functionType.rfind('(') - 1)
		);
		boost::split(returnTypes, returnTypesString, boost::is_any_of(","));
		for (auto &returnType : returnTypes)
			result.push_back(CreateStateType(returnType));
	}
	return result;
}

void State::set(std::string const &name, StateType const &type)
{
	(*this)[name] = type;
	// print();
}

void State::set(std::string const &name, AST_Type const &type)
{
	(*this)[name] = CreateStateType(type);
	// print();
}

StateType &State::get(std::string const &name)
{
	return (*this)[name];
}

void State::print()
{
	std::cout << " --- state ---" << std::endl;
	for (auto &state : (*this))
	{
		std::cout << TypeAsString(state.second) << " " << state.first << " = " << ValueAsString(state.second)
				  << std::endl;
	}
	std::cout << " -------------" << std::endl;
}

} // namespace soltest

} // namespace dev
