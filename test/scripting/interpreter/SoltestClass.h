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
/** @file SoltestClass.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTCLASS_H
#define SOLIDITY_SOLTESTCLASS_H

#include "SoltestStack.h"
#include "StateType.h"

namespace dev
{

namespace soltest
{

class SoltestClass
{
public:
	std::vector<dev::soltest::StateType> call(dev::soltest::MemberAccess const &member,
											  std::vector<dev::soltest::AST_Type> const &arguments);

	u256 setChainParams(u256 a, u256 b);

private:
	bool setChainParams(std::vector<dev::soltest::StateType> const &arguments,
						std::vector<dev::soltest::StateType> &returns);
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTCLASS_H
