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
/** @file SoltestTypes.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "SoltestTypes.h"

namespace dev
{

namespace soltest
{

Type CreateType(std::string const &_typename)
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

Type LexicalCast(Type const &prototype, std::string const &_string)
{
	Type result;
	try
	{
		if (prototype.type() == typeid(bool))
			result = boost::lexical_cast<bool>(_string);
		else if (prototype.type() == typeid(int8_t))
			result = boost::lexical_cast<int8_t>(_string);
		else if (prototype.type() == typeid(int16_t))
			result = boost::lexical_cast<int16_t>(_string);
		else if (prototype.type() == typeid(int32_t))
			result = boost::lexical_cast<int32_t>(_string);
		else if (prototype.type() == typeid(int64_t))
			result = boost::lexical_cast<int64_t>(_string);
		else if (prototype.type() == typeid(s256))
			result = boost::lexical_cast<s256>(_string);
		else if (prototype.type() == typeid(uint8_t))
			result = boost::lexical_cast<uint8_t>(_string);
		else if (prototype.type() == typeid(uint16_t))
			result = boost::lexical_cast<uint16_t>(_string);
		else if (prototype.type() == typeid(uint32_t))
			result = boost::lexical_cast<uint32_t>(_string);
		else if (prototype.type() == typeid(uint64_t))
			result = boost::lexical_cast<uint64_t>(_string);
		else if (prototype.type() == typeid(u256))
			result = boost::lexical_cast<u256>(_string);
		else if (prototype.type() == typeid(h160))
			result = h160(_string);
		else if (prototype.type() == typeid(h256))
			result = h256(_string);
		else if (prototype.type() == typeid(Address))
			result = Address(u160(_string));
		else if (prototype.type() == typeid(Contract))
			result = Contract(_string);
		else if (prototype.type() == typeid(std::string))
			result = _string;
	}
	catch (...)
	{
		result = Empty();
	}
	return result;
}

std::string TypeAsString(Type const &type)
{
	if (type.type() == typeid(Empty))
		return "Empty";
	else if (type.type() == typeid(bool))
		return "bool";
	else if (type.type() == typeid(int8_t))
		return "int8_t";
	else if (type.type() == typeid(int16_t))
		return "int16_t";
	else if (type.type() == typeid(int32_t))
		return "int32_t";
	else if (type.type() == typeid(int64_t))
		return "int64_t";
	else if (type.type() == typeid(s256))
		return "s256";
	else if (type.type() == typeid(uint8_t))
		return "uint8_t";
	else if (type.type() == typeid(uint16_t))
		return "uint16_t";
	else if (type.type() == typeid(uint32_t))
		return "uint32_t";
	else if (type.type() == typeid(uint64_t))
		return "uint64_t";
	else if (type.type() == typeid(u160))
		return "u160";
	else if (type.type() == typeid(u256))
		return "u256";
	else if (type.type() == typeid(Address))
		return "Address";
	else if (type.type() == typeid(Contract))
		return "Contract";
	else if (type.type() == typeid(std::string))
		return "string";
	return "?";
}

std::string ValueAsString(Type const &type)
{
	std::stringstream result;
	if (type.type() == typeid(Empty))
		return "(Empty)";
	else if (type.type() == typeid(bool))
		result << boost::get<bool>(type);
	else if (type.type() == typeid(int8_t))
		result << boost::get<int8_t>(type) << std::hex << " = 0x" << boost::get<int8_t>(type);
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
	else if (type.type() == typeid(Address))
		result << boost::get<Address>(type).value() << std::hex << " = 0x" << boost::get<Address>(type).value();
	else if (type.type() == typeid(Contract))
		result << boost::get<Contract>(type).name;
	else if (type.type() == typeid(std::string))
		result << boost::get<std::string>(type);
	return result.str();
}

Type BinaryExpression(Type const &left, std::string const &_operator, Type const &right)
{
	try
	{
		if (left.type() == right.type() && left.type() == typeid(bool))
			return BinaryExpression_Impl(boost::get<bool>(left), _operator, boost::get<bool>(right));
		else if (left.type() == right.type() && left.type() == typeid(int8_t))
			return BinaryExpression_Impl(boost::get<int8_t>(left), _operator, boost::get<int8_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(int16_t))
			return BinaryExpression_Impl(boost::get<int16_t>(left), _operator, boost::get<int16_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(int32_t))
			return BinaryExpression_Impl(boost::get<int32_t>(left), _operator, boost::get<int32_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(int64_t))
			return BinaryExpression_Impl(boost::get<int64_t>(left), _operator, boost::get<int64_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(s256))
			return BinaryExpression_Impl(boost::get<s256>(left), _operator, boost::get<s256>(right));
		else if (left.type() == right.type() && left.type() == typeid(uint8_t))
			return BinaryExpression_Impl(boost::get<uint8_t>(left), _operator, boost::get<uint8_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(uint16_t))
			return BinaryExpression_Impl(boost::get<uint16_t>(left), _operator, boost::get<uint16_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(uint32_t))
			return BinaryExpression_Impl(boost::get<uint32_t>(left), _operator, boost::get<uint32_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(uint64_t))
			return BinaryExpression_Impl(boost::get<uint64_t>(left), _operator, boost::get<uint64_t>(right));
		else if (left.type() == right.type() && left.type() == typeid(u160))
			return BinaryExpression_Impl(boost::get<u160>(left), _operator, boost::get<u160>(right));
		else if (left.type() == right.type() && left.type() == typeid(u256))
			return BinaryExpression_Impl(boost::get<u256>(left), _operator, boost::get<u256>(right));
//		else if (left.type() == right.type() && left.type() == typeid(Address))
//			return BinaryExpression_Impl(boost::get<Address>(left), _operator, boost::get<Address>(right));
//		else if (left.type() == right.type() && left.type() == typeid(Contract))
//			return BinaryExpression_Impl(boost::get<Contract>(left), _operator, boost::get<Contract>(right));
//		else if (left.type() == right.type() && left.type() == typeid(std::string))
//			return BinaryExpression_Impl(boost::get<std::string>(left), _operator, boost::get<std::string>(right));
	}
	catch (...)
	{
	}
	return Empty();
}

} // namespace soltest

} // namespace dev