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
/** @file StateType.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_STATETYPE_H
#define SOLIDITY_STATETYPE_H

#include "SoltestAST.h"

#include <boost/variant.hpp>

namespace dev
{

namespace soltest
{

typedef boost::variant<
	Empty,

	bool, /* double, // not  yet implemented in solidity */
	int8_t, int16_t, int32_t, int64_t, s256, // todo: int128_t
	uint8_t, uint16_t, uint32_t, uint64_t, u160, u256, // todo: uint128_t
	std::string,

	Address, Contract
> StateType;

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_STATETYPE_H
