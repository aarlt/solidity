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
 * JSON interface for the solidity compiler to be used from Javascript.
 */

#include <libsolc/libsolc.h>
#include <libdevcore/Common.h>
#include <libdevcore/JSON.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/Version.h>

#include <string>

#include "license.h"

using namespace std;
using namespace dev;
using namespace solidity;

namespace
{

ReadCallback::Callback wrapReadCallback(CStyleReadFileCallback _readCallback = nullptr)
{
	ReadCallback::Callback readCallback;
	if (_readCallback)
	{
		readCallback = [=](string const& _path)
		{
			char* contents_c = nullptr;
			char* error_c = nullptr;
			_readCallback(_path.c_str(), &contents_c, &error_c);
			ReadCallback::Result result;
			result.success = true;
			if (!contents_c && !error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = "File not found.";
			}
			if (contents_c)
			{
				result.success = true;
				result.responseOrErrorMessage = string(contents_c);
				free(contents_c);
			}
			if (error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = string(error_c);
				free(error_c);
			}
			return result;
		};
	}
	return readCallback;
}

/// Translates a gas value as a string to a JSON number or null
Json gasToJson(Json const& _value)
{
	if (_value.is_object())
	{
		Json ret = Json::object();
		for (auto const& sig: _value.items())
			ret[sig.key()] = gasToJson(_value[sig.key()]);
		return ret;
	}

	if (_value == "infinite")
		return Json(nullptr);

	u256 value(_value.get<string>());
	if (value > std::numeric_limits<Json::number_integer_t>::max())
		return Json(nullptr);
	else
		return Json(static_cast<Json::number_integer_t>(value));
}

Json translateGasEstimates(Json const& estimates)
{
	Json output(Json::object());

	if (estimates["creation"].is_object())
	{
		Json creation(Json::array());
		creation[0] = gasToJson(estimates["creation"]["executionCost"]);
		creation[1] = gasToJson(estimates["creation"]["codeDepositCost"]);
		output["creation"] = creation;
	}
	else
		output["creation"] = Json::object();
	output["external"] = gasToJson(estimates.value("external", Json::object()));
	output["internal"] = gasToJson(estimates.value("internal", Json::object()));

	return output;
}

string compile(StringMap const& _sources, bool _optimize, CStyleReadFileCallback _readCallback)
{
	/// create new JSON input format
	Json input = Json::object();
	input["language"] = "Solidity";
	input["sources"] = Json::object();
	for (auto const& source: _sources)
	{
		input["sources"][source.first] = Json::object();
		input["sources"][source.first]["content"] = source.second;
	}
	input["settings"] = Json::object();
	input["settings"]["optimizer"] = Json::object();
	input["settings"]["optimizer"]["enabled"] = _optimize;
	input["settings"]["optimizer"]["runs"] = 200;

	// Enable all SourceUnit-level outputs.
	input["settings"]["outputSelection"]["*"][""][0] = "*";
	// Enable all Contract-level outputs.
	input["settings"]["outputSelection"]["*"]["*"][0] = "*";

	StandardCompiler compiler(wrapReadCallback(_readCallback));
	Json ret = compiler.compile(input);

	/// transform JSON to match the old format
	// {
	//   "errors": [ "Error 1", "Error 2" ],
	//   "sourceList": [ "sourcename1", "sourcename2" ],
	//   "sources": {
	//     "sourcename1": {
	//       "AST": {}
	//     }
	//   },
	//   "contracts": {
	//     "Contract1": {
	//       "interface": "[...abi...]",
	//       "bytecode": "ff0011...",
	//       "runtimeBytecode": "ff0011",
	//       "opcodes": "PUSH 1 POP STOP",
	//       "metadata": "{...metadata...}",
	//       "functionHashes": {
	//         "test(uint256)": "11ff2233"
	//       },
	//       "gasEstimates": {
	//         "creation": [ 224, 42000 ],
	//         "external": {
	//           "11ff2233": null,
	//           "3322ff11": 1234
	//         },
	//         "internal": {
	//         }
	//       },
	//       "srcmap" = "0:1:2",
	//       "srcmapRuntime" = "0:1:2",
	//       "assembly" = {}
	//     }
	//   }
	// }
	Json output = Json::object();

	if (ret.find("errors") != ret.end())
	{
		output["errors"] = Json::array();
		for (auto const& error: ret["errors"])
			output["errors"].emplace_back(
				!error["formattedMessage"].empty() ? error["formattedMessage"] : error["message"]
			);
	}

	output["sourceList"] = Json::array();
	for (auto const& source: _sources)
		output["sourceList"].emplace_back(source.first);

	if (ret.find("sources") != ret.end())
	{
		output["sources"] = Json::object();
		for (auto const& sourceName: ret["sources"].items())
		{
			output["sources"][sourceName.key()] = Json::object();
			output["sources"][sourceName.key()]["AST"] = ret["sources"][sourceName.key()]["legacyAST"];
		}
	}

	if (ret.find("contracts") != ret.end())
	{
		output["contracts"] = Json::object();
		for (auto const& sourceName: ret["contracts"].items())
			for (auto const& contractName: ret["contracts"][sourceName.key()].items())
			{
				Json contractInput = ret["contracts"][sourceName.key()][contractName.key()];
				Json contractOutput = Json::object();
				contractOutput["interface"] = jsonCompactPrint(contractInput["abi"]);
				contractOutput["metadata"] = contractInput["metadata"];
				contractOutput["functionHashes"] = contractInput["evm"]["methodIdentifiers"];
				contractOutput["gasEstimates"] = translateGasEstimates(contractInput["evm"]["gasEstimates"]);
				contractOutput["assembly"] = contractInput["evm"]["legacyAssembly"];
				contractOutput["bytecode"] = contractInput["evm"]["bytecode"]["object"];
				contractOutput["opcodes"] = contractInput["evm"]["bytecode"]["opcodes"];
				contractOutput["srcmap"] = contractInput["evm"]["bytecode"]["sourceMap"];
				contractOutput["runtimeBytecode"] = contractInput["evm"]["deployedBytecode"]["object"];
				contractOutput["srcmapRuntime"] = contractInput["evm"]["deployedBytecode"]["sourceMap"];
				output["contracts"][sourceName.key() + ":" + contractName.key()] = contractOutput;
			}
	}

	try
	{
		return jsonCompactPrint(output);
	}
	catch (...)
	{
		return "{\"errors\":[\"Unknown error while generating JSON.\"]}";
	}
}

string compileMulti(string const& _input, bool _optimize, CStyleReadFileCallback _readCallback = nullptr)
{
	string errors;
	Json input;
	if (!jsonParseStrict(_input, input, &errors))
	{
		Json jsonErrors(Json::array());
		jsonErrors.emplace_back("Error parsing input JSON: " + errors);
		Json output(Json::object());
		output["errors"] = jsonErrors;
		return jsonCompactPrint(output);
	}
	else
	{
		StringMap sources;
		Json jsonSources = input["sources"];
		if (jsonSources.is_object())
			for (auto const& sourceName: jsonSources.items())
				sources[sourceName.key()] = jsonSources[sourceName.key()].get<string>();
		return compile(sources, _optimize, _readCallback);
	}
}

string compileSingle(string const& _input, bool _optimize)
{
	StringMap sources;
	sources[""] = _input;
	return compile(sources, _optimize, nullptr);
}


string compileStandardInternal(string const& _input, CStyleReadFileCallback _readCallback = nullptr)
{
	StandardCompiler compiler(wrapReadCallback(_readCallback));
	return compiler.compile(_input);
}

}

static string s_outputBuffer;

extern "C"
{
extern char const* license()
{
	static string fullLicenseText = otherLicenses + licenseText;
	return fullLicenseText.c_str();
}
extern char const* version()
{
	return VersionString.c_str();
}
extern char const* compileJSON(char const* _input, bool _optimize)
{
	s_outputBuffer = compileSingle(_input, _optimize);
	return s_outputBuffer.c_str();
}
extern char const* compileJSONMulti(char const* _input, bool _optimize)
{
	s_outputBuffer = compileMulti(_input, _optimize);
	return s_outputBuffer.c_str();
}
extern char const* compileJSONCallback(char const* _input, bool _optimize, CStyleReadFileCallback _readCallback)
{
	s_outputBuffer = compileMulti(_input, _optimize, _readCallback);
	return s_outputBuffer.c_str();
}
extern char const* compileStandard(char const* _input, CStyleReadFileCallback _readCallback)
{
	s_outputBuffer = compileStandardInternal(_input, _readCallback);
	return s_outputBuffer.c_str();
}
}
