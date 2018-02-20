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
/** @file SoltestTypes.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTTYPES_H
#define SOLIDITY_SOLTESTTYPES_H

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <memory>
#include <vector>

#define TYPEINFO(name) \
    #name << ": " << ValueAsString(name) << " (" << TypeAsString(name) << ")"

namespace dev
{

namespace soltest
{

class Empty
{
};

struct Contract
{
	explicit Contract(std::string const &_name) : name(_name)
	{
		// nothing to do here
	}
	std::string name;
};

struct Address
{
public:
	Address() : m_value()
	{
	}
	Address(u160 const &_value) : m_value(_value)
	{
	}

	u160 value() const
	{
		return m_value;
	}

private:
	u160 m_value;
};

typedef boost::variant<
	Empty, bool,
	int8_t, int16_t, int32_t, int64_t, s256, // todo: int128_t
	uint8_t, uint16_t, uint32_t, uint64_t, u160, u256, // todo: uint128_t
	std::string, Address, Contract
> Type;

typedef typename std::shared_ptr<Type> TypePtr;

Type CreateType(std::string const &_typename);

Type LexicalCast(Type const &prototype, std::string const &_string);

std::string TypeAsString(Type const &type);

std::string ValueAsString(Type const &type);

Type EvaluateBinaryExpression(std::string const &_operator, Type left, Type right);

template<typename T>
Type LexicalCast(std::string const &_string)
{
	Type result;
	try
	{
		result = boost::lexical_cast<T>(_string);
	}
	catch (...)
	{
		result = Empty();
	}
	return result;
}

template<typename T>
class Stack : public std::vector<T>
{
public:
	void push(T element)
	{
		this->emplace_back(element);
		print();
	}
	T pop()
	{
		if (this->size() > 0)
		{
			T result = this->back();
			this->pop_back();
			print();
			return result;
		}
		else
		{
			return Empty();
		}
	}

	void print()
	{
		std::cout << " --- stack ---" << std::endl;
		if (!this->empty())
		{
			std::cout << "   size: " << this->size() << std::endl;
			std::cout << "   next: " << ValueAsString(this->back()) << " (" << TypeAsString(this->back()) << ")"
					  << std::endl;
			std::cout << " -------------" << std::endl;
		}
		for (auto iter = this->rbegin(); iter != this->rend(); ++iter)
		{
			std::cout << ValueAsString(*iter) << " (" << TypeAsString(*iter) << ")" << std::endl;
		}
		if (this->empty())
		{
			std::cout << "   { empty }" << std::endl;
		}
		std::cout << " -------------" << std::endl;
	}

};

template<typename T>
T BinaryExpression_Impl(T _left, std::string const &_operator, T _right)
{
	if (_operator == "+")
		return _left + _right;
	else if (_operator == "-")
		return _left - _right;
	else if (_operator == "*")
		return _left * _right;
	else if (_operator == "/")
		return _left / _right;

	throw std::exception();
}

Type BinaryExpression(Type const &left, std::string const &_operator, Type const &right);

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTTYPES_H
