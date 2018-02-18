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
/** @file SoltestASTChecker.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTASTCHECKER_H
#define SOLIDITY_SOLTESTASTCHECKER_H

#include <libdevcore/Common.h>
#include <libevmasm/SourceLocation.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/LValue.h>
#include <libsolidity/interface/Exceptions.h>

#include <functional>
#include <memory>
#include <boost/noncopyable.hpp>

namespace dev
{

namespace soltest
{

/// Only a limited subset of the solidity language is allowed in soltest.
/// This class is responsible to check the AST to determine not supported usages of solidity within soltest.
class SoltestASTChecker : private dev::solidity::ASTConstVisitor
{
public:
	explicit SoltestASTChecker(dev::solidity::SourceUnit const &sourceUnit, std::string const &testcaseName)
		: m_testcaseName(testcaseName), m_interresting(false), m_valid(true)
	{
		sourceUnit.accept(*this);
	}

	bool isValid() const
	{
		return this->m_valid;
	}

	std::string const &errors() const
	{
		return m_errors;
	}

protected:
	bool visit(dev::solidity::FunctionDefinition const &_node) override;

	bool visit(dev::solidity::IfStatement const &_node) override;

	bool visit(dev::solidity::WhileStatement const &_node) override;

	bool visit(dev::solidity::ForStatement const &_node) override;

	bool visit(dev::solidity::Continue const &_node) override;

	bool visit(dev::solidity::InlineAssembly const &_node) override;

	bool visit(dev::solidity::Break const &_node) override;

	bool visit(dev::solidity::Return const &_node) override;

	bool visit(dev::solidity::Throw const &_node) override;

	bool visit(dev::solidity::Conditional const &_node) override;
private:
	std::string m_testcaseName;
	bool m_interresting;
	bool m_valid;
	std::string m_errors;
};

bool IsCorrectAST(dev::solidity::SourceUnit const &sourceUnit, std::string const &testcaseName, std::string &errors);

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTASTCHECKER_H
