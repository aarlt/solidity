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

	The Implementation originally from https://msdn.microsoft.com/en-us/library/windows/desktop/aa365592(v=vs.85).aspx
*/
/** @file SoltestSession.cpp
 * Low-level IPC communication between the test framework and the Ethereum node.
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * based on test/RPCSession.h, written by Dimtiry Khokhlov <dimitry@ethdev.com>
 * and test/libsolidity/SolidityExecutionFramework.h, written by Christian <c@ethdev.com>
 * @date 2018
 */

#include "SoltestSession.h"

#include <libdevcore/CommonData.h>

#include <libdevcore/JSON.h>

#include <test/RPCSession.h>

#include <string>
#include <cstdio>
#include <thread>
#include <chrono>

#include <cstdlib>
#include <boost/test/framework.hpp>
#include <libdevcore/CommonIO.h>
#include <test/ExecutionFramework.h>
#include <test/scripting/interpreter/contract/Contract.h>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace dev;

namespace dev
{

namespace soltest
{

SoltestSession &SoltestSession::instance(const string &_path)
{
	static SoltestSession session(_path);
	BOOST_REQUIRE_EQUAL(session.m_ipcSocket.path(), _path);
	return session;
}

string SoltestSession::eth_getCode(string const &_address, string const &_blockNumber)
{
	return rpcCall("eth_getCode", {quote(_address), quote(_blockNumber)}).asString();
}

Json::Value SoltestSession::eth_getBlockByNumber(string const &_blockNumber, bool _fullObjects)
{
	// NOTE: to_string() converts bool to 0 or 1
	return rpcCall("eth_getBlockByNumber", {quote(_blockNumber), _fullObjects ? "true" : "false"});
}

Json::Value SoltestSession::eth_getAccounts() {
	return rpcCall("eth_accounts");
}

SoltestSession::TransactionReceipt SoltestSession::eth_getTransactionReceipt(string const &_transactionHash)
{
	TransactionReceipt receipt;
	Json::Value const result = rpcCall("eth_getTransactionReceipt", {quote(_transactionHash)});
	BOOST_REQUIRE(!result.isNull());
	receipt.gasUsed = result["gasUsed"].asString();
	receipt.contractAddress = result["contractAddress"].asString();
	receipt.blockNumber = result["blockNumber"].asString();
	for (auto const &log: result["logs"])
	{
		LogEntry entry;
		entry.address = log["address"].asString();
		entry.data = log["data"].asString();
		for (auto const &topic: log["topics"])
			entry.topics.push_back(topic.asString());
		receipt.logEntries.push_back(entry);
	}
	return receipt;
}

string SoltestSession::eth_sendTransaction(TransactionData const &_td)
{
	return rpcCall("eth_sendTransaction", {_td.toJson()}).asString();
}

string SoltestSession::eth_call(TransactionData const &_td, string const &_blockNumber)
{
	return rpcCall("eth_call", {_td.toJson(), quote(_blockNumber)}).asString();
}

string SoltestSession::eth_sendTransaction(string const &_transaction)
{
	return rpcCall("eth_sendTransaction", {_transaction}).asString();
}

string SoltestSession::eth_getBalance(string const &_address, string const &_blockNumber)
{
	string address = (_address.length() == 20) ? "0x" + _address : _address;
	return rpcCall("eth_getBalance", {quote(address), quote(_blockNumber)}).asString();
}

string SoltestSession::eth_getStorageRoot(string const &_address, string const &_blockNumber)
{
	string address = (_address.length() == 20) ? "0x" + _address : _address;
	return rpcCall("eth_getStorageRoot", {quote(address), quote(_blockNumber)}).asString();
}

void SoltestSession::personal_unlockAccount(string const &_address, string const &_password, int _duration)
{
	BOOST_REQUIRE_MESSAGE(
		rpcCall("personal_unlockAccount", {quote(_address), quote(_password), to_string(_duration)}),
		"Error unlocking account " + _address
	);
}

void SoltestSession::sendMessage(dev::soltest::Contract &_contract,
								 bytes const &_data,
								 bool _isCreation,
								 u256 const &_value)
{
	/*
	if (m_showMessages)
	{
		if (_isCreation)
			cout << "CREATE " << m_sender.hex() << ":" << endl;
		else
			cout << "CALL   " << m_sender.hex() << " -> " << m_contractAddress.hex() << ":" << endl;
		if (_value > 0)
			cout << " value: " << _value << endl;
		cout << " in:      " << toHex(_data) << endl;
	}
	 */
	SoltestSession::TransactionData d;
	d.data = "0x" + toHex(_data);
	d.from = "0x" + _contract.account().hex();
	d.gas = "0x500000";
	d.gasPrice = "0x" + u256("1").str();
	d.value = "0x" + _value.str();
	(void) _value;
	if (!_isCreation)
	{
		d.to = "0x" + dev::toString(_contract.address());
		BOOST_REQUIRE(this->eth_getCode(d.to, "latest").size() > 2);
		// Use eth_call to get the output
		bytes output = fromHex(this->eth_call(d, "latest"), WhenError::Throw);
	}

	std::string before = eth_getBlockByNumber("latest", false)["number"].asString();
	std::string txHash;
	std::string current(before);
	while (before == current)
	{
		txHash = this->eth_sendTransaction(d);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		current = eth_getBlockByNumber("latest", false)["number"].asString();
	}

	SoltestSession::TransactionReceipt receipt(this->eth_getTransactionReceipt(txHash));

	u256 blockNumber = u256(receipt.blockNumber);
	(void) blockNumber;

	if (_isCreation)
	{
		h160 contractAddress(receipt.contractAddress);
		_contract.setAddress(contractAddress);
		BOOST_REQUIRE(_contract.address());
		string code = this->eth_getCode("0x" + _contract.address().hex(), "latest");
		bytes output = fromHex(code, WhenError::Throw);
	}

	/*
	if (m_showMessages)
	{
		cout << " out:     " << toHex(m_output) << endl;
		cout << " tx hash: " << txHash << endl;
	}
	*/

	/*
	m_gasUsed = u256(receipt.gasUsed);
	m_logs.clear();
	for (auto const &log: receipt.logEntries)
	{
		LogEntry entry;
		entry.address = Address(log.address);
		for (auto const &topic: log.topics)
			entry.topics.push_back(h256(topic));
		entry.data = fromHex(log.data, WhenError::Throw);
		m_logs.push_back(entry);
	}
	 */
}

string SoltestSession::personal_newAccount(string const &_password)
{
	string addr = rpcCall("personal_newAccount", {quote(_password)}).asString();
	BOOST_TEST_MESSAGE("Created account " + addr);
	return addr;
}

Json::Value SoltestSession::rpcCall(string const &_methodName, vector<string> const &_args, bool _canFail)
{
	string request = "{\"jsonrpc\":\"2.0\",\"method\":\"" + _methodName + "\",\"params\":[";
	for (size_t i = 0; i < _args.size(); ++i)
	{
		request += _args[i];
		if (i + 1 != _args.size())
			request += ", ";
	}

	request += "],\"id\":" + to_string(m_rpcSequence) + "}";
	++m_rpcSequence;

	BOOST_TEST_MESSAGE("Request: " + request);
	string reply = m_ipcSocket.sendRequest(request);
	BOOST_TEST_MESSAGE("Reply: " + reply);

	Json::Value result;
	BOOST_REQUIRE(jsonParseStrict(reply, result));

	if (result.isMember("error"))
	{
		if (_canFail)
			return Json::Value();

		BOOST_FAIL("Error on JSON-RPC call: " + result["error"]["message"].asString());
	}
	return result["result"];
}

SoltestSession::SoltestSession(const string &_path) :
	m_ipcSocket(_path)
{
}

string SoltestSession::TransactionData::toJson() const
{
	Json::Value json;
	json["from"] = (from.length() == 20) ? "0x" + from : from;
	if (!to.empty())
	{
		json["to"] = (to.length() == 20 || to == "") ? "0x" + to : to;
	}
	json["gas"] = gas;
	json["gasprice"] = gasPrice;
	json["value"] = value;
	json["data"] = data;
	return jsonCompactPrint(json);
}

} // namespace soltest

} // namespace dev