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
/** @file RemoteContract.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#ifndef SOLIDITY_REMOTECONTRACT_H
#define SOLIDITY_REMOTECONTRACT_H

#include "Contract.h"

namespace dev
{

namespace soltest
{

class RemoteContract : public dev::soltest::Contract
{
public:
	RemoteContract(std::string const &type) : dev::soltest::Contract(type)
	{
	}

	bool construct(StateTypes &arguments);
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_REMOTECONTRACT_H
