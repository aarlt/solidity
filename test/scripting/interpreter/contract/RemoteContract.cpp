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
/** @file RemoteContract.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include <test/scripting/interpreter/SoltestState.h>
#include "RemoteContract.h"

namespace dev
{

namespace soltest
{

bool dev::soltest::RemoteContract::construct(dev::soltest::StateTypes &arguments)
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	for (auto &arg : arguments)
	{
		std::cout << " - " << TypeAsString(arg) << " " << ValueAsString(arg) << std::endl;
	}
	return true;
}

} // namespace soltest

} // namespace dev
