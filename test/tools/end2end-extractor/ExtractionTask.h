#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <libsolutil/Common.h>
#include <libsolutil/FixedHash.h>
#include <test/libsolidity/util/BytesUtils.h>

class ExtractionTask
{
	using bytes = solidity::bytes;
	using u256 = solidity::u256;
	using h256 = solidity::util::h256;
	using Address = solidity::util::h160;

  public:
	ExtractionTask() = default;
	explicit ExtractionTask(std::string _name, std::function<void(void)> _task)
	    : m_name(std::move(_name)), m_task(std::move(_task))
	{
	}

	void analyse() const { m_task(); }

	[[nodiscard]] const std::string &name() const { return m_name; }

	bool operator<(const ExtractionTask &right) const { return m_name < right.m_name; }

	bytes compileAndRunWithoutCheck(std::string const &_sourceCode,
	                                u256 const &_value,
	                                std::string const &_contractName,
	                                bytes const &_arguments,
	                                std::map<std::string, Address> const &_libraryAddresses
	                                = std::map<std::string, Address>())
	{
		(void) _sourceCode;
		(void) _value;
		(void) _contractName;
		(void) _arguments;
		(void) _libraryAddresses;

		m_sources.emplace_back(_sourceCode);
		return bytes();
	}

	void alsoViaYul() { m_alsoViaYul = true; }

	void extract()
	{
		std::cout << m_name << ".sol" << std::endl;
		for (auto &source : m_sources)
			std::cout << source << std::endl;
		for (auto &expectation : m_expectations)
			std::cout << expectation << std::endl;
		std::cout << std::endl << std::endl;
	}

	void extractionNotPossible(const std::string &_reason)
	{
		m_extractable = false;
		m_reasons.insert(_reason);
	}

	bool extractable() { return m_extractable; }

	[[nodiscard]] std::set<std::string> reasons() const { return m_reasons; }

	void addExpectation(const std::string &_sig, const std::string &_parameters, const std::string &_result)
	{
		std::stringstream o;
		o << "// " + _sig;
		if (!_parameters.empty())
			o << ": " + _parameters;
		o << " -> " + _result;

		m_expectations.emplace_back(o.str());
	}

  private:
	std::string m_name;
	std::function<void(void)> m_task;
	std::vector<std::string> m_sources;
	bool m_extractable{true};
	bool m_alsoViaYul{false};
	std::set<std::string> m_reasons;
	std::vector<std::string> m_expectations;
};
