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
/** @file SoltestAsserts.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTASSERTS_H
#define SOLIDITY_SOLTESTASSERTS_H

#include <libsolidity/ast/AST.h>

#define SOLTEST_TEST_TOOL_IMPL(frwd_type, P, FILE, LINE, assertion_descr, TL, CT, ARGS)     \
do {                                                                                        \
    BOOST_TEST_PASSPOINT();                                                                 \
    ::boost::test_tools::tt_detail::                                                        \
    BOOST_PP_IF( frwd_type, report_assertion, check_frwd ) (                                \
        BOOST_JOIN( BOOST_TEST_TOOL_PASS_PRED, frwd_type )( P, ARGS ),                      \
        BOOST_TEST_LAZY_MSG( assertion_descr ),                                             \
        FILE,                                                                               \
        static_cast<std::size_t>(LINE),                                                     \
        ::boost::test_tools::tt_detail::TL,                                                 \
        ::boost::test_tools::tt_detail::CT                                                  \
        BOOST_JOIN( BOOST_TEST_TOOL_PASS_ARGS, frwd_type )( ARGS ) );                       \
} while( ::boost::test_tools::tt_detail::dummy_cond() )                                     \
/**/

#define SOLTEST_REQUIRE_MESSAGE(P, FILE, LINE, M)  \
    SOLTEST_TEST_TOOL_IMPL( 2, (P), FILE, LINE, M, REQUIRE, CHECK_MSG, _ )

inline void ExtractSoltestLocation(dev::solidity::ASTNode const &_node,
								   std::string const &source,
								   std::string &item,
								   size_t &line)
{
	dev::SourceLocation const &location(_node.location());
	item = source.substr(location.start, location.end - location.start);
	std::string currentLine(source.substr(location.start, source.substr(location.start).find("\n")));
	std::string currentLineNumber(currentLine.substr(currentLine.find("//_soltest_line:") + 16));
	line = static_cast<std::size_t>(std::atoi(currentLineNumber.c_str()));
}

#endif //SOLIDITY_SOLTESTASSERTS_H
