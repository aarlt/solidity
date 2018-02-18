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
/** @file SoltestExecutor.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "SoltestExecutor.h"

#include "SoltestASTChecker.h"

#include <sstream>
#include <libsolidity/ast/ASTPrinter.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace dev
{

namespace soltest
{

SoltestExecutor::SoltestExecutor(dev::solidity::SourceUnit const &sourceUnit,
								 std::string const &contract,
								 std::string const &filename,
								 uint32_t line)
	: m_sourceUnit(sourceUnit), m_contract(contract), m_filename(filename), m_line(line)
{
	(void) m_sourceUnit;
	(void) m_contract;
	(void) m_filename;
	(void) m_line;
}

bool SoltestExecutor::execute(std::string const &testcase, std::string &errors)
{
	dev::solidity::FunctionDefinition const *functionToExecute =
		dev::soltest::FindFunction(m_sourceUnit, testcase);
	if (functionToExecute != nullptr)
	{
		dev::solidity::ASTPrinter printer(*functionToExecute);
		printer.print(std::cout);
		functionToExecute->accept(*this);
		if (!m_errors.empty())
		{
			std::stringstream errorStream;
			errorStream << m_errors << ": " << m_contract << " " << testcase << " " << m_filename << ":" << m_line;
			errors = errorStream.str();
		}
		return m_errors.empty();
	}
	return false;
}

bool SoltestExecutor::visit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement)
{
	return ASTConstVisitor::visit(_variableDeclarationStatement);
}

void SoltestExecutor::endVisit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement)
{
	if (m_stack.size() == 1)
	{
		TypedData variable = m_stack.back();
		m_stack.pop_back();
		m_state[variable.first] = variable.second;
	}
	else if (m_stack.size() == 2)
	{
		TypedData literal = m_stack.back();
		m_stack.pop_back();
		TypedData variable = m_stack.back();
		m_stack.pop_back();
		try
		{
			m_state[variable.first] = lexical_cast(variable.second, literal.first);
		}
		catch (...)
		{
			std::stringstream errors;
			errors << "lexical cast failed for '" << variable.first << " = " << literal.first << "("
				   << literal.second << ")'" << std::endl;
			m_errors += errors.str();
		}
	}
	std::cout << std::endl;
	for (auto &v : m_state)
	{
		std::cout << v.first << " : " << v.second << " @ " << v.second.type().name() << std::endl;
	}
	std::cout << std::endl;
	ASTConstVisitor::endVisit(_variableDeclarationStatement);
}

bool SoltestExecutor::visit(dev::solidity::VariableDeclaration const &_variableDeclaration)
{
	std::string typeName(_variableDeclaration.annotation().type->toString());
	std::string variableName(_variableDeclaration.name());
	if (boost::starts_with(typeName, "contract"))
	{
		m_contracts[variableName] = typeName;
	}
	else
	{
		if (typeName == "bool")
			m_stack.emplace_back(std::make_pair(variableName, bool()));
		else if (typeName == "int8")
			m_stack.emplace_back(std::make_pair(variableName, int8_t()));
		else if (typeName == "int16")
			m_stack.emplace_back(std::make_pair(variableName, int16_t()));
		else if (typeName == "int32")
			m_stack.emplace_back(std::make_pair(variableName, int32_t()));
		else if (typeName == "int64")
			m_stack.emplace_back(std::make_pair(variableName, int64_t()));
		else if (typeName == "int256")
			m_stack.emplace_back(std::make_pair(variableName, s256()));
		else if (typeName == "uint8")
			m_stack.emplace_back(std::make_pair(variableName, uint8_t()));
		else if (typeName == "uint16")
			m_stack.emplace_back(std::make_pair(variableName, uint16_t()));
		else if (typeName == "uint32")
			m_stack.emplace_back(std::make_pair(variableName, uint32_t()));
		else if (typeName == "uint64")
			m_stack.emplace_back(std::make_pair(variableName, uint64_t()));
		else if (typeName == "uint256")
			m_stack.emplace_back(std::make_pair(variableName, u256()));
		else if (typeName == "address")
			m_stack.emplace_back(std::make_pair(variableName, h160()));
		else if (typeName == "string")
			m_stack.emplace_back(std::make_pair(variableName, std::string()));
	}
	return ASTConstVisitor::visit(_variableDeclaration);
}

bool SoltestExecutor::visit(dev::solidity::Literal const &_literal)
{
	std::string typeName(_literal.annotation().type->toString());
	m_stack.emplace_back(std::make_pair(_literal.value(), typeName));
	return ASTConstVisitor::visit(_literal);
}

SoltestExecutor::StateType SoltestExecutor::lexical_cast(StateType const &_type, std::string const &_string)
{
	if (_type.type() == typeid(bool))
		return boost::lexical_cast<bool>(_string);
	else if (_type.type() == typeid(int8_t))
		return boost::lexical_cast<int8_t>(_string);
	else if (_type.type() == typeid(int16_t))
		return boost::lexical_cast<int16_t>(_string);
	else if (_type.type() == typeid(int32_t))
		return boost::lexical_cast<int32_t>(_string);
	else if (_type.type() == typeid(int64_t))
		return boost::lexical_cast<int64_t>(_string);
	else if (_type.type() == typeid(s256))
		return boost::lexical_cast<s256>(_string);
	else if (_type.type() == typeid(uint8_t))
		return boost::lexical_cast<uint8_t>(_string);
	else if (_type.type() == typeid(uint16_t))
		return boost::lexical_cast<uint16_t>(_string);
	else if (_type.type() == typeid(uint32_t))
		return boost::lexical_cast<uint32_t>(_string);
	else if (_type.type() == typeid(uint64_t))
		return boost::lexical_cast<uint64_t>(_string);
	else if (_type.type() == typeid(u256))
		return boost::lexical_cast<u256>(_string);
	else if (_type.type() == typeid(h160))
		return h160(_string);
	else if (_type.type() == typeid(std::string))
		return _string;

	throw boost::bad_lexical_cast();
}

void SoltestExecutor::endVisit(dev::solidity::VariableDeclaration const &_variableDeclaration)
{
	ASTConstVisitor::endVisit(_variableDeclaration);
}

bool SoltestExecutor::visit(dev::solidity::Assignment const &_assignment)
{
	return ASTConstVisitor::visit(_assignment);
}

bool SoltestExecutor::visit(dev::solidity::TupleExpression const &_tuple)
{
	return ASTConstVisitor::visit(_tuple);
}

bool SoltestExecutor::visit(dev::solidity::UnaryOperation const &_unaryOperation)
{
	return ASTConstVisitor::visit(_unaryOperation);
}

bool SoltestExecutor::visit(dev::solidity::BinaryOperation const &_binaryOperation)
{
	return ASTConstVisitor::visit(_binaryOperation);
}

bool SoltestExecutor::visit(dev::solidity::FunctionCall const &_functionCall)
{
	return ASTConstVisitor::visit(_functionCall);
}

bool SoltestExecutor::visit(dev::solidity::NewExpression const &_newExpression)
{
	return ASTConstVisitor::visit(_newExpression);
}

bool SoltestExecutor::visit(dev::solidity::MemberAccess const &_memberAccess)
{
	return ASTConstVisitor::visit(_memberAccess);
}

bool SoltestExecutor::visit(dev::solidity::IndexAccess const &_indexAccess)
{
	return ASTConstVisitor::visit(_indexAccess);
}
} // namespace soltest

} // namespace dev