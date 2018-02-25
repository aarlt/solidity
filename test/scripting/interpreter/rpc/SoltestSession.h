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
/** @file SoltestSession.h
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * based on test/RPCSession.h, written by Dimtiry Khokhlov <dimitry@ethdev.com>
 * and test/libsolidity/SolidityExecutionFramework.h, written by Christian <c@ethdev.com>
 * @date 2018
 */

#ifndef SOLIDITY_SOLTESTSESSION_H
#define SOLIDITY_SOLTESTSESSION_H

#include <json/value.h>

#include <test/RPCSession.h>

#include <boost/noncopyable.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <cstdio>
#include <map>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

namespace dev
{

namespace soltest
{

class Contract;

class SoltestSession : public boost::noncopyable
{
public:
	struct TransactionData
	{
		std::string from;
		std::string to;
		std::string gas;
		std::string gasPrice;
		std::string value;
		std::string data;

		std::string toJson() const;
	};

	struct LogEntry
	{
		std::string address;
		std::vector<std::string> topics;
		std::string data;
	};

	struct TransactionReceipt
	{
		std::string gasUsed;
		std::string contractAddress;
		std::vector<LogEntry> logEntries;
		std::string blockNumber;
	};

	static SoltestSession &instance(std::string const &_path);

	std::string eth_getCode(std::string const &_address, std::string const &_blockNumber);
	Json::Value eth_getBlockByNumber(std::string const &_blockNumber, bool _fullObjects);
	std::string eth_call(TransactionData const &_td, std::string const &_blockNumber);
	TransactionReceipt eth_getTransactionReceipt(std::string const &_transactionHash);
	std::string eth_sendTransaction(TransactionData const &_td);
	Json::Value eth_getAccounts();
	std::string eth_sendTransaction(std::string const &_transaction);
	std::string eth_getBalance(std::string const &_address, std::string const &_blockNumber);
	std::string eth_getStorageRoot(std::string const &_address, std::string const &_blockNumber);

	std::string personal_newAccount(std::string const &_password);
	void personal_unlockAccount(std::string const &_address, std::string const &_password, int _duration);

	void sendMessage(dev::soltest::Contract& _contract, bytes const& _data, bool _isCreation, u256 const& _value = 0);

	Json::Value rpcCall(std::string const &_methodName,
						std::vector<std::string> const &_args = std::vector<std::string>(),
						bool _canFail = false);

private:
	explicit SoltestSession(std::string const &_path);

	inline std::string quote(std::string const &_arg) { return "\"" + _arg + "\""; }
	/// Parse std::string replacing keywords to values
	void parseString(std::string &_string, std::map<std::string, std::string> const &_varMap);

	IPCSocket m_ipcSocket;
	size_t m_rpcSequence = 1;
	unsigned m_maxMiningTime = 6000000; // 600 seconds
	unsigned m_sleepTime = 10; // 10 milliseconds
	unsigned m_successfulMineRuns = 0;

	std::vector<std::string> m_accounts;
};

} // namespace soltest

} // namespace dev

#endif //SOLIDITY_SOLTESTSESSION_H