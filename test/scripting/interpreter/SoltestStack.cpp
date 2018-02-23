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
/** @file SoltestStack.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "SoltestStack.h"

#include "SoltestState.h"

namespace dev
{

namespace soltest
{

std::string TypeAsString(AST_Type const &type)
{
	if (type.type() == typeid(Empty))
		return "Empty";
	else if (type.type() == typeid(TypeId))
		return "TypeId";

	else if (type.type() == typeid(Literal))
		return "Literal";
	else if (type.type() == typeid(VariableDeclaration))
		return "VariableDeclaration";
	else if (type.type() == typeid(BinaryOperation))
		return "BinaryOperation";
	else if (type.type() == typeid(Identifier))
		return "Identifier";
	else if (type.type() == typeid(MemberAccess))
		return "MemberAccess";

	else if (type.type() == typeid(StateType))
	{
		if (boost::get<StateType>(type).type() == typeid(bool))
			return "bool";
		else if (boost::get<StateType>(type).type() == typeid(int8_t))
			return "int8_t";
		else if (boost::get<StateType>(type).type() == typeid(int16_t))
			return "int16_t";
		else if (boost::get<StateType>(type).type() == typeid(int32_t))
			return "int32_t";
		else if (boost::get<StateType>(type).type() == typeid(int64_t))
			return "int64_t";
		else if (boost::get<StateType>(type).type() == typeid(s256))
			return "s256";
		else if (boost::get<StateType>(type).type() == typeid(uint8_t))
			return "uint8_t";
		else if (boost::get<StateType>(type).type() == typeid(uint16_t))
			return "uint16_t";
		else if (boost::get<StateType>(type).type() == typeid(uint32_t))
			return "uint32_t";
		else if (boost::get<StateType>(type).type() == typeid(uint64_t))
			return "uint64_t";
		else if (boost::get<StateType>(type).type() == typeid(u160))
			return "u160";
		else if (boost::get<StateType>(type).type() == typeid(u256))
			return "u256";
		else if (boost::get<StateType>(type).type() == typeid(std::string))
			return "string";

		else if (boost::get<StateType>(type).type() == typeid(Address))
			return "Address";
		else if (boost::get<StateType>(type).type() == typeid(Contract))
			return "Contract";
	}

	return "?";
}

std::string ValueAsString(AST_Type const &type)
{
	(void) type;
	std::stringstream result;
	if (type.type() == typeid(Empty))
		return "(Empty)";
	else if (type.type() == typeid(TypeId))
		result << boost::get<TypeId>(type).asString();

	else if (type.type() == typeid(Literal))
		result << boost::get<Literal>(type).asString();
	else if (type.type() == typeid(VariableDeclaration))
		result << boost::get<VariableDeclaration>(type).asString();
	else if (type.type() == typeid(BinaryOperation))
		result << boost::get<BinaryOperation>(type).asString();
	else if (type.type() == typeid(Identifier))
		result << boost::get<Identifier>(type).asString();
	else if (type.type() == typeid(MemberAccess))
		result << boost::get<MemberAccess>(type).asString();

	else if (type.type() == typeid(StateType))
	{
		if (boost::get<StateType>(type).type() == typeid(bool))
			result << boost::get<bool>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(int8_t))
			result << boost::get<int8_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<int8_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(int16_t))
			result << boost::get<int16_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<int16_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(int32_t))
			result << boost::get<int32_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<int32_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(int64_t))
			result << boost::get<int64_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<int64_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(s256))
			result << boost::get<s256>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<s256>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(uint8_t))
			result << boost::get<uint8_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<uint8_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(uint16_t))
			result << boost::get<uint16_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<uint16_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(uint32_t))
			result << boost::get<uint32_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<uint32_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(uint64_t))
			result << boost::get<uint64_t>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<uint64_t>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(u160))
			result << boost::get<u160>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<u160>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(u256))
			result << boost::get<u256>(boost::get<StateType>(type)) << std::hex << " = 0x"
				   << boost::get<u256>(boost::get<StateType>(type));
		else if (boost::get<StateType>(type).type() == typeid(std::string))
			result << boost::get<std::string>(boost::get<StateType>(type));

		else if (boost::get<StateType>(type).type() == typeid(Address))
			result << boost::get<Address>(boost::get<StateType>(type)).value() << std::hex << " = 0x"
				   << boost::get<Address>(boost::get<StateType>(type)).value();
		else if (boost::get<StateType>(type).type() == typeid(Contract))
			result << boost::get<Contract>(boost::get<StateType>(type)).asString();
	}
	return result.str();
}

Literal Evaluate(Literal const &left, std::string const &op, Literal const &right)
{
	std::string value;
	if (left.category == right.category)
	{
		using s512= boost::multiprecision::number<
			boost::multiprecision::cpp_int_backend<
				512, 512, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void
			> >;
		s512 leftValue;
		s512 rightValue;
		if (boost::starts_with(left.value, "0x"))
		{
			std::stringstream ss;
			ss << std::hex << left.value;
			ss >> leftValue;
		}
		else
			leftValue = boost::lexical_cast<s512>(left.value);

		if (boost::starts_with(right.value, "0x"))
		{
			std::stringstream ss;
			ss << std::hex << right.value;
			ss >> rightValue;
		}
		else
			rightValue = boost::lexical_cast<s512>(right.value);
		s512 result;
		if (op == "+")
		{
			result = leftValue + rightValue;
		}
		else if (op == "-")
		{
			result = leftValue - rightValue;
		}
		else if (op == "/")
		{
			result = leftValue / rightValue;
		}
		else if (op == "*")
		{
			result = leftValue * rightValue;
		}
		else if (op == "<")
		{
			result = leftValue < rightValue;
		}
		else if (op == ">")
		{
			result = leftValue > rightValue;
		}
		else if (op == ">=")
		{
			result = leftValue >= rightValue;
		}
		else if (op == "<=")
		{
			result = leftValue <= rightValue;
		}
		else if (op == "==")
		{
			result = leftValue == rightValue;
		}
		std::stringstream resultStream;
		resultStream << result;
		value = resultStream.str();
	}
	return Literal(left.category, value);
}

Literal Evaluate(StateType const &type)
{
	std::stringstream result;
	result << RawValueAsString(type);
	return Literal(solidity::Type::Category::RationalNumber, result.str());
}

void Stack::push(AST_Type element)
{
	this->emplace_back(element);
}

AST_Type Stack::pop()
{
	if (!this->empty())
	{
		AST_Type result = this->back();
		this->pop_back();
		print();
		return result;
	}
	else
	{
		return StateType(Empty());
	}
}

AST_Type Stack::first()
{
	if (!this->empty())
	{
		AST_Type result = *begin();
		return result;
	}
	else
	{
		return StateType(Empty());
	}
}

Stack &Stack::operator<<(AST_Type const &_value)
{
	push(_value);
	print();
	return *this;
}

void Stack::print()
{
	std::cout << " --- stack ---" << std::endl;
	if (!this->empty())
	{
		std::cout << "   size: " << this->size() << std::endl;
		std::cout << "   next: " << " [ " << TypeAsString(this->back()) << " ] " << ValueAsString(this->back())
				  << std::endl;
		std::cout << " -------------" << std::endl;
	}
	for (auto iter = this->rbegin(); iter != this->rend(); ++iter)
	{
		std::cout << "[ " << TypeAsString(*iter) << " ] " << ValueAsString(*iter) << std::endl;
	}
	if (this->empty())
	{
		std::cout << "   { empty }" << std::endl;
	}
	std::cout << " -------------" << std::endl;
}

} // namespace soltest

} // namespace dev

