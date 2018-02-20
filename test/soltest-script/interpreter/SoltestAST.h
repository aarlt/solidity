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
/** @file SoltestAST.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTAST_H
#define SOLIDITY_SOLTESTAST_H

#include <libsolidity/ast/AST.h>

namespace dev
{

namespace soltest
{

class Empty
{

};

struct TypeId
{
	TypeId() : type_id(std::string(typeid(Empty).name()))
	{
	}
	template<class T>
	TypeId(T const &instance)
	{
		(void) instance;
		type_id = std::string(typeid(T).name());
	}
	std::string asString() const
	{
		std::stringstream stream;
		stream << "type_id = '" << type_id << "'";
		return stream.str();
	}
	std::string type_id;
};

struct Contract
{
	explicit Contract(std::string const &_type) : type(_type)
	{
	}
	std::string asString() const
	{
		std::stringstream stream;
		stream << "type = '" << type << "'";
		return stream.str();
	}
	std::string type;
};

struct Address
{
public:
	Address() : m_value()
	{
	}
	Address(u160 _value) : m_value(_value)
	{
	}

	u160 value() const
	{
		return m_value;
	}

	std::string asString() const
	{
		std::stringstream stream;
		stream << "value = " << m_value;
		return stream.str();
	}
private:
	u160 m_value;
};

struct Literal
{
	Literal(dev::solidity::Type::Category _category, std::string const &_value) : category(_category), value(_value)
	{
	}
	std::string asString() const
	{
		std::stringstream stream;
		std::string categoryStr;
		switch (category)
		{
		case dev::solidity::Type::Category::RationalNumber:
			categoryStr = "RationalNumber";
			break;
		case dev::solidity::Type::Category::Bool:
			categoryStr = "Bool";
			break;
		case dev::solidity::Type::Category::Integer:
			categoryStr = "Integer";
			break;
		case dev::solidity::Type::Category::StringLiteral:
			categoryStr = "StringLiteral";
			break;
		default:
			categoryStr = "?";
		}
		stream << "category = " << categoryStr << ", value = '" << value << "'";
		return stream.str();
	}
	dev::solidity::Type::Category category;
	std::string value;
};

struct VariableDeclaration
{
	VariableDeclaration(std::string const &_name, std::string const &_type) : name(_name), type(_type)
	{
	}
	std::string asString() const
	{
		std::stringstream stream;
		stream << "name = " << name << ", type = '" << type << "'";
		return stream.str();
	}
	std::string name;
	std::string type;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTAST_H
