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
#include <libsolidity/ast/AST.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#define TYPEID(T) \
    std::string(typeid(T).name())

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

void SoltestExecutor::print(dev::solidity::ASTNode const &node)
{
	dev::solidity::ASTPrinter printer(node);
	printer.print(std::cout);
}

void SoltestExecutor::endVisit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement)
{
	Type value;
	if (boost::get<std::string>(m_stack.back()) == TYPEID(dev::solidity::VariableDeclaration))
	{
		m_stack.pop();
		std::string name(boost::get<std::string>(m_stack.pop()));
		m_state[name] = m_stack.pop();
	}
	else if (boost::get<std::string>(m_stack.back()) == TYPEID(dev::solidity::Literal))
	{
		m_stack.pop();
		std::string literal_type(boost::get<std::string>(m_stack.pop()));
		std::string literal_value(boost::get<std::string>(m_stack.pop()));
		if (literal_type == "int_const")
		{
			if (boost::get<std::string>(m_stack.pop()) == TYPEID(dev::solidity::VariableDeclaration))
			{
				std::string name(boost::get<std::string>(m_stack.pop()));
				m_state[name] = LexicalCast(m_stack.pop(), boost::get<std::string>(literal_value));
			}
		}
		else
		{
			std::string message("literal type '" + literal_type + "' not implemented.");
			BOOST_ASSERT_MSG(false, message.c_str());
		}
	}
	else
	{
		std::string message("[ VariableDeclarationStatement -> (VariableDeclaration | Literal) ] GOT "
								+ boost::get<std::string>(m_stack.back()));
		BOOST_ASSERT_MSG(false, message.c_str());
	}

	ASTConstVisitor::endVisit(_variableDeclarationStatement);
}

void SoltestExecutor::endVisit(dev::solidity::VariableDeclaration const &_variableDeclaration)
{
	std::string name(_variableDeclaration.name());
	std::string type(_variableDeclaration.annotation().type->toString());
	m_stack.push(dev::soltest::CreateType(type));
	m_stack.push(name);
	m_stack.push(TYPEID(dev::solidity::VariableDeclaration));
	ASTConstVisitor::endVisit(_variableDeclaration);
}

void SoltestExecutor::endVisit(dev::solidity::Literal const &_literal)
{
	std::string value_and_type(_literal.annotation().type->toString());
	std::string type(value_and_type.substr(0, value_and_type.find(' ')));
	std::string value(value_and_type.substr(value_and_type.find(' ') + 1));
	m_stack.push(value);
	m_stack.push(type);
	m_stack.push(TYPEID(dev::solidity::Literal));
	ASTConstVisitor::endVisit(_literal);
}

void SoltestExecutor::endVisit(dev::solidity::Assignment const &_assignment)
{
	// print(_assignment);
	// print();
	std::string type(_assignment.annotation().type->toString());
	m_stack.pop();
	Type value(m_stack.pop());
	if (boost::get<std::string>(m_stack.back()) == TYPEID(dev::solidity::Identifier))
	{
		m_stack.pop();
		std::string assign_to(boost::get<std::string>(m_stack.pop()));
		m_state[assign_to] = value;
	}
	ASTConstVisitor::endVisit(_assignment);
}

void SoltestExecutor::endVisit(dev::solidity::BinaryOperation const &_binaryOperation)
{
	std::string operation(Token::toString(_binaryOperation.getOperator()));
	Type right;
	std::string right_ast_type;
	Type left;
	std::string left_ast_type;

	print(_binaryOperation);
	right_ast_type = boost::get<std::string>(m_stack.pop());
	if (right_ast_type == TYPEID(dev::solidity::Identifier))
	{
		right = m_stack.pop();
	}
	else if (right_ast_type == TYPEID(dev::solidity::Literal))
	{
		std::string literal_type(boost::get<std::string>(m_stack.pop()));
		std::string literal_value(boost::get<std::string>(m_stack.pop()));
		if (literal_type == "int_const")
		{
			m_stack.pop();
			right = LexicalCast(m_stack.pop(), boost::get<std::string>(literal_value));
		}
		else
		{
			std::string message("literal type '" + literal_type + "' not implemented.");
			BOOST_ASSERT_MSG(false, message.c_str());
		}
	}
	else if (right_ast_type == TYPEID(dev::solidity::BinaryOperation))
	{
		right = m_stack.pop();
	}

	left_ast_type = boost::get<std::string>(m_stack.pop());
	if (left_ast_type == TYPEID(dev::solidity::Identifier))
	{
		left = m_stack.pop();
	}
	else if (left_ast_type == TYPEID(dev::solidity::Literal))
	{
		std::string literal_type(boost::get<std::string>(m_stack.pop()));
		std::string literal_value(boost::get<std::string>(m_stack.pop()));
		if (literal_type == "int_const")
		{
			left = LexicalCast(m_stack.pop(), boost::get<std::string>(literal_value));
		}
		else
		{
			std::string message("literal type '" + literal_type + "' not implemented.");
			BOOST_ASSERT_MSG(false, message.c_str());
		}
	}
	else if (left_ast_type == TYPEID(dev::solidity::BinaryOperation))
	{
		left = m_stack.pop();
	}

	if (left_ast_type == TYPEID(dev::solidity::Identifier))
	{
		left = m_state[boost::get<std::string>(left)];
	}
	if (right_ast_type == TYPEID(dev::solidity::Identifier))
	{
		right = m_state[boost::get<std::string>(right)];
	}
	m_stack.push(BinaryExpression(left, operation, right));
	m_stack.push(TYPEID(dev::solidity::BinaryOperation));
	print(_binaryOperation);
	ASTConstVisitor::endVisit(_binaryOperation);
}

void SoltestExecutor::endVisit(dev::solidity::Identifier const &_identifier)
{
	std::string name(_identifier.name());
	std::string type(_identifier.annotation().type->toString());
	m_stack.push(name);
	m_stack.push(TYPEID(dev::solidity::Identifier));
	ASTConstVisitor::endVisit(_identifier);
}

void SoltestExecutor::endVisit(dev::solidity::TupleExpression const &_tuple)
{
	ASTConstVisitor::endVisit(_tuple);
}

void SoltestExecutor::endVisit(dev::solidity::UnaryOperation const &_unaryOperation)
{
	ASTConstVisitor::endVisit(_unaryOperation);
}

void SoltestExecutor::endVisit(dev::solidity::FunctionCall const &_functionCall)
{
	ASTConstVisitor::endVisit(_functionCall);
}

void SoltestExecutor::endVisit(dev::solidity::NewExpression const &_newExpression)
{
	ASTConstVisitor::endVisit(_newExpression);
}

void SoltestExecutor::endVisit(dev::solidity::MemberAccess const &_memberAccess)
{
	ASTConstVisitor::endVisit(_memberAccess);
}

void SoltestExecutor::endVisit(dev::solidity::IndexAccess const &_indexAccess)
{
	ASTConstVisitor::endVisit(_indexAccess);
}

} // namespace soltest

} // namespace dev