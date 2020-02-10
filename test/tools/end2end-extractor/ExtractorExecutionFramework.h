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
 * @date 2014
 * Framework for executing contracts and testing them using RPC.
 */

#pragma once

#include <test/Common.h>

#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/OptimiserSettings.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/Keccak256.h>
#include <test/libsolidity/util/BytesUtils.h>
#include <any>
#include <functional>

#include <fstream>
#include <sstream>

namespace solidity::test
{
using rational = boost::rational<bigint>;
/// An Ethereum address: 20 bytes.
/// @NOTE This is not endian-specific; it's just a bunch of bytes.
using Address = util::h160;

// The various denominations; here for ease of use where needed within code.
static const u256 wei = 1;
static const u256 shannon = u256("1000000000");
static const u256 szabo = shannon * 1000;
static const u256 finney = szabo * 1000;
static const u256 ether = finney * 1000;

class ExtractorExecutionFramework
{
  public:
	ExtractorExecutionFramework();
	explicit ExtractorExecutionFramework(langutil::EVMVersion _evmVersion);
	virtual ~ExtractorExecutionFramework() = default;

	bytes const &compileAndRunWithoutCheck(std::string const &_sourceCode,
	                                       u256 const &_value = 0,
	                                       std::string const &_contractName = "",
	                                       bytes const &_arguments = bytes(),
	                                       std::map<std::string, Address> const &_libraryAddresses
	                                       = std::map<std::string, Address>())
	{
		(void) _value;
		(void) _contractName;
		(void) _arguments;
		(void) _libraryAddresses;

		m_testContentStream << _sourceCode << std::endl;
		if (!m_alsoViaYul)
			m_testContentStream << "// ----" << std::endl;
		else
			m_testContentStream << "// ====" << std::endl;
		if (m_alsoViaYul)
		{
			m_testContentStream << "// compileViaYul: also" << std::endl;
			m_testContentStream << "// ----" << std::endl;
			m_alsoViaYul = false;
		}
		return m_output;
	}

	bytes const &compileAndRun(std::string const &_sourceCode,
	                           u256 const &_value = 0,
	                           std::string const &_contractName = "",
	                           bytes const &_arguments = bytes(),
	                           std::map<std::string, Address> const &_libraryAddresses
	                           = std::map<std::string, Address>())
	{
		compileAndRunWithoutCheck(_sourceCode, _value, _contractName, _arguments, _libraryAddresses);
		//		BOOST_REQUIRE(m_transactionSuccessful);
		//		BOOST_REQUIRE(!m_output.empty());
		return m_output;
	}

	bytes const &callFallbackWithValue(u256 const &_value)
	{
		sendMessage(bytes(), false, _value);
		return m_output;
	}

	bytes const &callFallback() { return callFallbackWithValue(0); }

	bytes const &callLowLevel(bytes const &_data, u256 const &_value)
	{
		sendMessage(_data, false, _value);
		return m_output;
	}

	bytes const &callContractFunctionWithValueNoEncoding(std::string _sig, u256 const &_value, bytes const &_arguments)
	{
		(void)_sig;
		(void)_value;
		(void)_arguments;
		m_testContentStream << _sig << ":";
		m_testContentStream << frontend::test::BytesUtils::formatString(_arguments);
		return m_output;
	}

	bytes const &callContractFunctionNoEncoding(std::string _sig, bytes const &_arguments)
	{
		return callContractFunctionWithValueNoEncoding(_sig, 0, _arguments);
	}

	template <class... Args>
	bytes const &callContractFunctionWithValue(std::string _sig, u256 const &_value, Args const &... _arguments)
	{
		return callContractFunctionWithValueNoEncoding(_sig, _value, string_encodeArgs(_arguments...));
	}

	template <class... Args> bytes const &callContractFunction(std::string _sig, Args const &... _arguments)
	{
		return callContractFunctionWithValue(_sig, 0, _arguments...);
	}

	template <class CppFunction, class... Args>
	void testContractAgainstCpp(std::string _sig, CppFunction const &_cppFunction, Args const &... _arguments)
	{
		m_testContentStream << "// " << _sig << ": ";
		Arguments(m_testContentStream, _arguments...);
		m_testContentStream << " -> " << callCpp(_cppFunction, _arguments...)
		                                                 << std::endl;
	}

	template <class CppFunction, class... Args>
	void testContractAgainstCppOnRange(std::string _sig,
	                                   CppFunction const &_cppFunction,
	                                   u256 const &_rangeStart,
	                                   u256 const &_rangeEnd)
	{
		for (u256 argument = _rangeStart; argument < _rangeEnd; ++argument)
			m_testContentStream << "// " << _sig << ": " << argument << " -> " << _cppFunction(argument) << std::endl;
	}

	static std::string createIsoltestCall(std::string expectation, std::string result);
	static std::pair<bool, std::string> compareAndCreateMessage(bytes const &_result, bytes const &_expectation);

	static bytes encode(bool _value) { return encode(uint8_t(_value)); }
	static bytes encode(int _value) { return encode(u256(_value)); }
	static bytes encode(size_t _value) { return encode(u256(_value)); }
	static bytes encode(char const *_value) { return encode(std::string(_value)); }
	static bytes encode(uint8_t _value) { return bytes(31, 0) + bytes{_value}; }
	static bytes encode(u256 const &_value) { return util::toBigEndian(_value); }
	/// @returns the fixed-point encoding of a rational number with a given
	/// number of fractional bits.
	static bytes encode(std::pair<rational, int> const &_valueAndPrecision)
	{
		rational const &value = _valueAndPrecision.first;
		int fractionalBits = _valueAndPrecision.second;
		return encode(u256((value.numerator() << fractionalBits) / value.denominator()));
	}
	static bytes encode(util::h256 const &_value) { return _value.asBytes(); }
	static bytes encode(bytes const &_value, bool _padLeft = true)
	{
		bytes padding = bytes((32 - _value.size() % 32) % 32, 0);
		return _padLeft ? padding + _value : _value + padding;
	}
	static bytes encode(std::string const &_value) { return encode(util::asBytes(_value), false); }
	template <class _T> static bytes encode(std::vector<_T> const &_value)
	{
		bytes ret;
		for (auto const &v : _value)
			ret += encode(v);
		return ret;
	}

	template <class FirstArg, class... Args>
	static bytes encodeArgs(FirstArg const &_firstArg, Args const &... _followingArgs)
	{
		return encode(_firstArg) + encodeArgs(_followingArgs...);
	}
	static bytes encodeArgs() { return bytes(); }

	//@todo might be extended in the future
	template <class Arg> static bytes encodeDyn(Arg const &_arg)
	{
		return encodeArgs(u256(0x20), u256(_arg.size()), _arg);
	}

	template <class FirstArg, class... Args>
	static bytes string_encodeDyn(FirstArg const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		o << _firstArg;
		if (sizeof...(_followingArgs))
			o << ", ";
		return frontend::test::BytesUtils::convertString(o.str()) + string_encodeDyn(_followingArgs...);
	}
	static bytes string_encodeDyn() { return bytes(); }

	bytes string_bytes() {
		return bytes();
	}

	bytes string_bytes(bytes b) {
		return b;
	}

	bytes string_fromHex(std::string _hex) {
		return frontend::test::BytesUtils::convertString(_hex);
	}

	template <class... Args>
	static bytes string_encodeArgs(std::vector<u256> const &_firstArg, Args const &... _followingArgs)
	{
		(void)_firstArg;
		std::stringstream o;
//		o << _firstArg;
		o << "?";
		if (sizeof...(_followingArgs))
			o << ", ";
		return frontend::test::BytesUtils::convertString(o.str()) + string_encodeArgs(_followingArgs...);
	}

	template <class FirstArg, class... Args>
	static bytes string_encodeArgs(FirstArg const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		o << _firstArg;
		if (sizeof...(_followingArgs))
			o << ", ";
		return frontend::test::BytesUtils::convertString(o.str()) + string_encodeArgs(_followingArgs...);
	}
	static bytes string_encodeArgs() { return bytes(); }


	u256 gasLimit() const;
	u256 gasPrice() const;
	u256 blockHash(u256 const &_blockNumber) const;
	u256 blockNumber() const;

	template <typename Range>
	static bytes encodeArray(bool _dynamicallySized, bool _dynamicallyEncoded, Range const &_elements)
	{
		bytes result;
		if (_dynamicallySized)
			result += encode(u256(_elements.size()));
		if (_dynamicallyEncoded)
		{
			u256 offset = u256(_elements.size()) * 32;
			std::vector<bytes> subEncodings;
			for (auto const &element : _elements)
			{
				result += encode(offset);
				subEncodings.emplace_back(encode(element));
				offset += subEncodings.back().size();
			}
			for (auto const &subEncoding : subEncodings)
				result += subEncoding;
		}
		else
			for (auto const &element : _elements)
				result += encode(element);
		return result;
	}

  private:
	template <class FirstArg, class... Args>
	static std::ostream &  Arguments(std::ostream & o, FirstArg const &_firstArg, Args const &... _followingArgs)
	{
		o << std::hex << "0x" << u256(_firstArg);
		if (sizeof...(_followingArgs))
			o << ", ";
		return Arguments(o, _followingArgs...);
	}
	static std::ostream & Arguments(std::ostream & o) { return o; }

	template <class CppFunction, class... Args>
	auto callCpp(CppFunction const &_cppFunction, Args const &... _arguments) ->
	    typename std::enable_if<std::is_void<decltype(_cppFunction(_arguments...))>::value, std::string>::type
	{
		_cppFunction(_arguments...);
		return "";
	}
	template <class CppFunction, class... Args>
	auto callCpp(CppFunction const &_cppFunction, Args const &... _arguments) ->
	    typename std::enable_if<!std::is_void<decltype(_cppFunction(_arguments...))>::value, std::string>::type

	{
		std::stringstream str;
		str << std::hex << "0x" << u256(_cppFunction(_arguments...));
		return str.str();
	}

  protected:
	void sendMessage(bytes const &_data, bool _isCreation, u256 const &_value = 0);
	void sendEther(Address const &_to, u256 const &_value);
	size_t currentTimestamp();
	size_t blockTimestamp(u256 _number);

	/// @returns the (potentially newly created) _ith address.
	Address account(size_t _i);

	u256 balanceAt(Address const &_addr);
	bool storageEmpty(Address const &_addr);
	bool addressHasCode(Address const &_addr);

	langutil::EVMVersion m_evmVersion;
	solidity::frontend::RevertStrings m_revertStrings = solidity::frontend::RevertStrings::Default;
	solidity::frontend::OptimiserSettings m_optimiserSettings = solidity::frontend::OptimiserSettings::minimal();
	bool m_showMessages = false;

	bool m_transactionSuccessful = true;
	Address m_sender = account(0);
	Address m_contractAddress;
	u256 const m_gasPrice = 100 * szabo;
	u256 const m_gas = 100000000;
	bytes m_output;
	u256 m_gasUsed;
	size_t m_blockNumber;

	std::ofstream m_testContentStream;
	bool m_alsoViaYul{false};

	void prepareTest(std::string const &name)
	{
		if (m_testContentStream.is_open())
			m_testContentStream.close();

		m_testContentStream = std::ofstream("/tmp/e2e/" + name + ".sol", std::ofstream::out | std::ofstream::trunc);
	}

	void setAlsoViaYul() { m_alsoViaYul = true; }
};

//#define ABI_CHECK(result, expectation) do { \
//	auto abiCheckResult = ExtractorExecutionFramework::compareAndCreateMessage((result), (expectation)); \
//	BOOST_CHECK_MESSAGE(abiCheckResult.first, abiCheckResult.second); \
//} while (0)
//

} // namespace solidity::test
