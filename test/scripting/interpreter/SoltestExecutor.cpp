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

#include <test/scripting/SoltestAsserts.h>

#include <libsolidity/ast/ASTPrinter.h>
#include <boost/test/unit_test.hpp>

#include <boost/variant.hpp>

namespace dev
{

namespace soltest
{

SoltestExecutor::SoltestExecutor(dev::soltest::SoltestSession &_rpc,
								 dev::solidity::CompilerStack &_compilerStack,
								 dev::solidity::SourceUnit const &sourceUnit,
								 std::string const &contract,
								 std::string const &filename,
								 std::string const &source,
								 uint32_t line)
	: m_rpc(_rpc),
	  m_sourceUnit(sourceUnit),
	  m_contract(contract),
	  m_filename(filename),
	  m_source(source),
	  m_line(line),
	  m_compilerStack(_compilerStack),
	  m_soltest(_rpc)
{
	m_state["soltest"] = m_soltest;
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

bool SoltestExecutor::visit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement)
{
	(void) _variableDeclarationStatement;
	// remove not needed function returns from stack
	m_stack.clear();
	return true;
}

void SoltestExecutor::endVisit(dev::solidity::VariableDeclarationStatement const &_variableDeclarationStatement)
{
	if (!m_stack.empty() && m_stack.back().type() == typeid(dev::soltest::VariableDeclaration))
	{
		AST_Type declaration = m_stack.pop();
		dev::soltest::VariableDeclaration
			variableDeclaration = boost::get<dev::soltest::VariableDeclaration>(declaration);
		m_state.set(variableDeclaration.name, CreateStateType(variableDeclaration.type));
	}

	if (!m_stack.empty() && m_stack.back().type() == typeid(dev::soltest::Literal))
	{
		AST_Type value = m_stack.pop();
		dev::soltest::Literal literal = boost::get<dev::soltest::Literal>(value);
		if (m_stack.back().type() == typeid(dev::soltest::VariableDeclaration))
		{
			AST_Type declaration = m_stack.pop();
			dev::soltest::VariableDeclaration
				variableDeclaration = boost::get<dev::soltest::VariableDeclaration>(declaration);
			m_state.set(variableDeclaration.name,
						LexicalCast(CreateStateType(variableDeclaration.type), literal.value));
		}
	}
	ASTConstVisitor::endVisit(_variableDeclarationStatement);
}

void SoltestExecutor::endVisit(dev::solidity::VariableDeclaration const &_variableDeclaration)
{
	solidity::TypePointer type = _variableDeclaration.annotation().type;
	m_stack << VariableDeclaration(_variableDeclaration.name(), type->toString());
	ASTConstVisitor::endVisit(_variableDeclaration);
}

void SoltestExecutor::endVisit(dev::solidity::Literal const &_literal)
{
	solidity::TypePointer type = _literal.annotation().type;
	m_stack << Literal(type->category(), _literal.value());
	ASTConstVisitor::endVisit(_literal);
}

void SoltestExecutor::endVisit(dev::solidity::Assignment const &_assignment)
{
	ASTConstVisitor::endVisit(_assignment);
}

void SoltestExecutor::endVisit(dev::solidity::BinaryOperation const &_binaryOperation)
{
	AST_Type right = m_stack.pop();
	AST_Type left = m_stack.pop();

	if (left.type() == typeid(dev::soltest::Identifier))
	{
		left = Evaluate(m_state[boost::get<dev::soltest::Identifier>(left).name]);
	}
	if (right.type() == typeid(dev::soltest::Identifier))
	{
		right = Evaluate(m_state[boost::get<dev::soltest::Identifier>(right).name]);
	}

	if (left.type() == typeid(dev::soltest::Literal) && right.type() == typeid(dev::soltest::Literal))
	{
		m_stack << Evaluate(
			boost::get<dev::soltest::Literal>(left),
			Token::toString(_binaryOperation.getOperator()),
			boost::get<dev::soltest::Literal>(right)
		);
	}
	ASTConstVisitor::endVisit(_binaryOperation);
}

void SoltestExecutor::endVisit(dev::solidity::Identifier const &_identifier)
{
	solidity::TypePointer type = _identifier.annotation().type;
	m_stack << Identifier(_identifier.name(), type->toString());
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
	std::string currentFunctionCall;
	size_t line;
	ExtractSoltestLocation(_functionCall, m_source, currentFunctionCall, line);

	BOOST_TEST_MESSAGE("- " + currentFunctionCall + "...");

	dev::soltest::AST_Types untyped_arguments;
	for (size_t i = 0; i < _functionCall.arguments().size(); ++i)
	{
		untyped_arguments.push_back(m_stack.pop());
	}
	if (m_stack.back().type() == typeid(dev::soltest::Identifier))
	{
		dev::soltest::Identifier identifier = boost::get<dev::soltest::Identifier>(m_stack.pop());
		dev::soltest::StateTypes
			arguments = CreateArgumentStateTypesFromFunctionType(identifier.type, untyped_arguments, m_state);
		if (identifier.name == "assert" && arguments.size() == 1)
		{
			BOOST_REQUIRE_MESSAGE(arguments[0].type() == typeid(bool), " assert argument need to be bool!");
			std::stringstream stream;
			stream << currentFunctionCall << " failed.";
			SOLTEST_REQUIRE_MESSAGE(
				boost::get<bool>(arguments[0]),
				m_filename.c_str(), line,
				stream.str()
			);
		}
	}
	else if (m_stack.back().type() == typeid(dev::soltest::MemberAccess))
	{
		dev::soltest::MemberAccess memberAccess = boost::get<dev::soltest::MemberAccess>(m_stack.pop());
		dev::soltest::Identifier identifier = boost::get<dev::soltest::Identifier>(m_stack.pop());
		dev::soltest::StateTypes
			arguments = CreateArgumentStateTypesFromFunctionType(memberAccess.type, untyped_arguments, m_state);
		if (boost::starts_with(identifier.type, "contract ") && boost::starts_with(memberAccess.type, "function "))
		{
			// contract function call
			BOOST_REQUIRE_MESSAGE(
				m_state[identifier.name].type() == typeid(dev::soltest::Contract),
				"No contract - this should not happen!"
			);
			std::reverse(arguments.begin(), arguments.end());
			dev::soltest::StateTypes
				results = CreateReturnStateTypesFromFunctionType(memberAccess.type);
			SOLTEST_REQUIRE_MESSAGE(
				boost::get<dev::soltest::Contract>(
					m_state[identifier.name]).call(m_soltest.account(), memberAccess.member, arguments, results),
				m_filename.c_str(), line,
				"Contract couldn't be called."
			);
			for (auto &result : results)
				if (result.type() == typeid(bool))
				{
					m_stack.push(
						dev::soltest::Literal(
							dev::solidity::Type::Category::Bool, RawValueAsString(result)
						)
					);
				}
				else
				{
					m_stack.push(
						dev::soltest::Literal(
							dev::solidity::Type::Category::RationalNumber, RawValueAsString(result)
						)
					);
				}
		}
	}
	else if (m_stack.back().type() == typeid(dev::soltest::NewExpression))
	{
		dev::soltest::NewExpression newExpression = boost::get<dev::soltest::NewExpression>(m_stack.pop());
		dev::soltest::VariableDeclaration
			variableDeclaration = boost::get<dev::soltest::VariableDeclaration>(m_stack.pop());

		dev::soltest::StateTypes
			arguments = CreateArgumentStateTypesFromFunctionType(newExpression.type, untyped_arguments, m_state);
		Contract contract(variableDeclaration.type, true, &m_rpc, &m_compilerStack);
		std::reverse(arguments.begin(), arguments.end());
		BOOST_REQUIRE_MESSAGE(contract.construct(m_soltest.account(), arguments), "Construction failed.");
		m_state[variableDeclaration.name] = contract;
	}

	BOOST_TEST_MESSAGE("- " + currentFunctionCall + "... done");

	ASTConstVisitor::endVisit(_functionCall);
}

void SoltestExecutor::endVisit(dev::solidity::NewExpression const &_newExpression)
{
	std::string type = _newExpression.annotation().type->toString();
	m_stack.push(dev::soltest::NewExpression(type));
	ASTConstVisitor::endVisit(_newExpression);
}

void SoltestExecutor::endVisit(dev::solidity::MemberAccess const &_memberAccess)
{
	solidity::TypePointer type = _memberAccess.annotation().type;
	m_stack << MemberAccess(_memberAccess.memberName(), type->toString());
	ASTConstVisitor::endVisit(_memberAccess);
}

void SoltestExecutor::endVisit(dev::solidity::IndexAccess const &_indexAccess)
{
	ASTConstVisitor::endVisit(_indexAccess);
}

} // namespace soltest

} // namespace dev