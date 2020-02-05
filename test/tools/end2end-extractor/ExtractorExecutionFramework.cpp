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
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Framework for executing contracts and testing them using RPC.
 */

#include "ExtractorExecutionFramework.h"

#include <libsolutil/CommonIO.h>

#include <boost/algorithm/string/replace.hpp>

#include <cstdlib>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::test;

ExtractorExecutionFramework::ExtractorExecutionFramework():
	ExtractorExecutionFramework(langutil::EVMVersion::berlin())
{
}

ExtractorExecutionFramework::ExtractorExecutionFramework(langutil::EVMVersion _evmVersion):
	m_evmVersion(_evmVersion),
	m_optimiserSettings(solidity::frontend::OptimiserSettings::minimal())
{
}

std::pair<bool, string> ExtractorExecutionFramework::compareAndCreateMessage(
	bytes const& _result,
	bytes const& _expectation
)
{
	if (_result == _expectation)
		return std::make_pair(true, std::string{});
	std::string message =
			"Invalid encoded data\n"
			"   Result                                                           Expectation\n";
	auto resultHex = boost::replace_all_copy(toHex(_result), "0", ".");
	auto expectedHex = boost::replace_all_copy(toHex(_expectation), "0", ".");
	for (size_t i = 0; i < std::max(resultHex.size(), expectedHex.size()); i += 0x40)
	{
		std::string result{i >= resultHex.size() ? string{} : resultHex.substr(i, 0x40)};
		std::string expected{i > expectedHex.size() ? string{} : expectedHex.substr(i, 0x40)};
		message +=
			(result == expected ? "   " : " X ") +
			result +
			std::string(0x41 - result.size(), ' ') +
			expected +
			"\n";
	}
	return make_pair(false, message);
}

u256 ExtractorExecutionFramework::gasLimit() const
{
	return 0;
}

u256 ExtractorExecutionFramework::gasPrice() const
{
	return 0;
}

u256 ExtractorExecutionFramework::blockHash(u256 const& _number) const
{
	(void)_number;
	return 0;
}

u256 ExtractorExecutionFramework::blockNumber() const
{
	return 0;
}

void ExtractorExecutionFramework::sendMessage(bytes const& _data, bool _isCreation, u256 const& _value)
{
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
//	evmc_message message = {};
//	message.input_data = _data.data();
//	message.input_size = _data.size();
//	message.sender = EVMHost::convertToEVMC(m_sender);
//	message.value = EVMHost::convertToEVMC(_value);
//
//	if (_isCreation)
//	{
//		message.kind = EVMC_CREATE;
//		message.destination = EVMHost::convertToEVMC(Address{});
//	}
//	else
//	{
//		message.kind = EVMC_CALL;
//		message.destination = EVMHost::convertToEVMC(m_contractAddress);
//	}
//	message.gas = m_gas.convert_to<int64_t>();
//
//	evmc::result result = m_evmHost->call(message);
//
//	m_output = bytes(result.output_data, result.output_data + result.output_size);
//	if (_isCreation)
//		m_contractAddress = EVMHost::convertFromEVMC(result.create_address);
//
//	m_gasUsed = m_gas - result.gas_left;
//	m_transactionSuccessful = (result.status_code == EVMC_SUCCESS);
//
//	if (m_showMessages)
//	{
//		cout << " out:     " << toHex(m_output) << endl;
//		cout << " result: " << size_t(result.status_code) << endl;
//		cout << " gas used: " << m_gasUsed.str() << endl;
//	}
}

void ExtractorExecutionFramework::sendEther(Address const& _addr, u256 const& _amount)
{
	(void)_addr;
	(void)_amount;
}

size_t ExtractorExecutionFramework::currentTimestamp()
{
	return 0;
}

size_t ExtractorExecutionFramework::blockTimestamp(u256 _block)
{
	if (_block > blockNumber())
		return 0;
	else
		return size_t((currentTimestamp() / blockNumber()) * _block);
}

Address ExtractorExecutionFramework::account(size_t _idx)
{
	return Address(h256(u256{"0x1212121212121212121212121212120000000012"} + _idx * 0x1000), Address::AlignRight);
}

bool ExtractorExecutionFramework::addressHasCode(Address const& _addr)
{
	(void)_addr;
	return 0;
}

size_t ExtractorExecutionFramework::numLogs() const
{
	return 0;
}

size_t ExtractorExecutionFramework::numLogTopics(size_t _logIdx) const
{
	(void)_logIdx;
	return 0;
}

h256 ExtractorExecutionFramework::logTopic(size_t _logIdx, size_t _topicIdx) const
{
	(void)_logIdx;
	(void)_topicIdx;
	return h256();
}

Address ExtractorExecutionFramework::logAddress(size_t _logIdx) const
{
	(void)_logIdx;
	return Address();
}

bytes ExtractorExecutionFramework::logData(size_t _logIdx) const
{
	(void)_logIdx;
	return bytes();
}

u256 ExtractorExecutionFramework::balanceAt(Address const& _addr)
{
	(void)_addr;
	return 0;
}

bool ExtractorExecutionFramework::storageEmpty(Address const& _addr)
{
	(void)_addr;
	return true;
}
