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
/** @file SoltestClass.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "SoltestContract.h"
#include "test/scripting/interpreter/SoltestState.h"

#include <boost/algorithm/string.hpp>

namespace dev
{

namespace soltest
{

SoltestContract::SoltestContract(dev::soltest::SoltestSession &_rpc)
	: dev::soltest::Contract("contract SoltestContract", false, &_rpc)
{
	registerContractMethod<SoltestContract, u256, u256, u256>(
		"setChainParams", *this, &SoltestContract::setChainParams
	);
	registerContractMethod<SoltestContract, void, std::string>(
		"setAccount", *this, &SoltestContract::setAccount
	);
	m_master = h160(m_rpc->eth_getAccounts()[0].asString());
	m_rpc->personal_unlockAccount("0x" + m_master.hex(), "", 0);
	m_accounts["master"] = m_master;
	m_account = m_master;
}

void SoltestContract::setAccount(std::string account)
{
	if (m_accounts.find(account) == m_accounts.end())
	{
		std::string newAccount = m_rpc->personal_newAccount(account);
		m_rpc->personal_unlockAccount(newAccount, account, 0);
		m_accounts[account] = h160(newAccount);
		// load newly created accounts with 1 ether
		m_rpc->sendEther(m_master, m_accounts[account], 1 * dev::soltest::ether);
	}
	m_account = m_accounts[account];
}

u256 SoltestContract::setChainParams(u256 a, u256 b)
{
	u256 result;
	result = a + b;
	std::cout << __PRETTY_FUNCTION__ << "(" << a << ", " << b << ") = " << result << std::endl;
	return result;
}

} // namespace soltest

} // namespace dev