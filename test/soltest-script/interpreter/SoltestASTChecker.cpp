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
/** @file SoltestASTChecker.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "SoltestASTChecker.h"

#include <test/soltest-script/SoltestTests.h>

namespace dev
{

namespace soltest
{

bool IsCorrectAST(dev::solidity::SourceUnit const &sourceUnit,
				  std::string const &testcaseName,
				  std::string &errors)
{
	dev::soltest::SoltestASTChecker checker(sourceUnit, testcaseName);
	errors = checker.errors();
	size_t pos = errors.rfind("\n");
	if (pos != std::string::npos)
	{
		errors.replace(pos, pos + 1, "");
	}
	return checker.isValid();
}

bool SoltestASTChecker::visit(dev::solidity::FunctionDefinition const &_node)
{
	m_interresting = _node.name() == dev::soltest::SoltestTests::NormalizeName(m_testcaseName);
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::IfStatement const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'If' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::ForStatement const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'For' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::Continue const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'Continue' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::InlineAssembly const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'InlineAssembly' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::Break const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'Break' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::Return const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'Return' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::Throw const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'Throw' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::Conditional const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'Conditional' statements not allowed in soltest.\n";
	return visitNode(_node);
}

bool SoltestASTChecker::visit(dev::solidity::WhileStatement const &_node)
{
	if (m_interresting)
	{
		m_valid = false;
	}
	m_errors += "Error: 'While' statements not allowed in soltest.\n";
	return visitNode(_node);
}

} // namespace soltest

} // namespace dev