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

#include <libdevcore/Common.h>
#include "Contract.h"
#include <tuple>

namespace dev
{

namespace soltest
{

class SetupContract : public dev::soltest::Contract
{
public:
	SetupContract() : dev::soltest::Contract("contract SetupContract")
	{
		registerContractMethod<
			SetupContract, u256, u256, u256>("setChainParams", *this, &SetupContract::setChainParams);
	}

	u256 setChainParams(u256 a, u256 b)
	{
		u256 result;
		result = a + b;
		std::cout << __PRETTY_FUNCTION__ << "(" << a << ", " << b << ") = " << result << std::endl;
		return result;
	}
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTCLASS_H
