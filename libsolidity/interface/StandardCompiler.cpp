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
 * @author Alex Beregszaszi
 * @date 2016
 * Standard JSON compiler interface.
 */

#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libevmasm/Instruction.h>
#include <libdevcore/JSON.h>
#include <libdevcore/SHA3.h>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

namespace {

Json formatError(
	bool _warning,
	string const& _type,
	string const& _component,
	string const& _message,
	string const& _formattedMessage = "",
	Json const& _sourceLocation = Json()
)
{
	Json error = Json::object();
	error["type"] = _type;
	error["component"] = _component;
	error["severity"] = _warning ? "warning" : "error";
	error["message"] = _message;
	error["formattedMessage"] = (_formattedMessage.length() > 0) ? _formattedMessage : _message;
	if (_sourceLocation.is_object())
		error["sourceLocation"] = _sourceLocation;
	return error;
}

Json formatFatalError(string const& _type, string const& _message)
{
	Json output = Json::object();
	output["errors"] = Json::array();
	output["errors"].emplace_back(formatError(false, _type, "general", _message));
	return output;
}

Json formatErrorWithException(
	Exception const& _exception,
	bool const& _warning,
	string const& _type,
	string const& _component,
	string const& _message,
	function<Scanner const&(string const&)> const& _scannerFromSourceName
)
{
	string message;
	string formattedMessage = SourceReferenceFormatter::formatExceptionInformation(_exception, _type, _scannerFromSourceName);

	// NOTE: the below is partially a copy from SourceReferenceFormatter
	SourceLocation const* location = boost::get_error_info<errinfo_sourceLocation>(_exception);

	if (string const* description = boost::get_error_info<errinfo_comment>(_exception))
		message = ((_message.length() > 0) ? (_message + ":") : "") + *description;
	else
		message = _message;

	Json sourceLocation;
	if (location && location->sourceName)
	{
		sourceLocation["file"] = *location->sourceName;
		sourceLocation["start"] = location->start;
		sourceLocation["end"] = location->end;
	}

	return formatError(_warning, _type, _component, message, formattedMessage, sourceLocation);
}

set<string> requestedContractNames(Json const& _outputSelection)
{
	set<string> names;
	for (auto const& sourceName: _outputSelection.items())
	{
		for (auto const& contractName: _outputSelection[sourceName.key()].items())
		{
			/// Consider the "all sources" shortcuts as requesting everything.
			if (contractName.key() == "*" || contractName.key() == "")
				return set<string>();
			names.insert((sourceName.key() == "*" ? "" : sourceName.key()) + ":" + contractName.key());
		}
	}
	return names;
}

/// Returns true iff @a _hash (hex with 0x prefix) is the Keccak256 hash of the binary data in @a _content.
bool hashMatchesContent(string const& _hash, string const& _content)
{
	try
	{
		return dev::h256(_hash) == dev::keccak256(_content);
	}
	catch (dev::BadHexCharacter const&)
	{
		return false;
	}
}

StringMap createSourceList(Json const& _input)
{
	StringMap sources;
	Json const& jsonSources = _input["sources"];
	if (jsonSources.is_object())
		for (auto const& sourceName: jsonSources.items())
			sources[sourceName.key()] = jsonSources[sourceName.key()]["content"].get<string>();
	return sources;
}

bool isArtifactRequested(Json const& _outputSelection, string const& _artifact)
{
	for (auto const& artifact: _outputSelection)
		/// @TODO support sub-matching, e.g "evm" matches "evm.assembly"
		if (artifact == "*" || artifact == _artifact)
			return true;
	return false;
}

///
/// @a _outputSelection is a JSON object containining a two-level hashmap, where the first level is the filename,
/// the second level is the contract name and the value is an array of artifact names to be requested for that contract.
/// @a _file is the current file
/// @a _contract is the current contract
/// @a _artifact is the current artifact name
///
/// @returns true if the @a _outputSelection has a match for the requested target in the specific file / contract.
///
/// In @a _outputSelection the use of '*' as a wildcard is permitted.
///
/// @TODO optimise this. Perhaps flatten the structure upfront.
///
bool isArtifactRequested(Json const& _outputSelection, string const& _file, string const& _contract, string const& _artifact)
{
	if (!_outputSelection.is_object())
		return false;

	for (auto const& file: { _file, string("*") })
		if (_outputSelection.find(file) != _outputSelection.end() && _outputSelection[file].is_object())
		{
			/// For SourceUnit-level targets (such as AST) only allow empty name, otherwise
			/// for Contract-level targets try both contract name and wildcard
			vector<string> contracts{ _contract };
			if (!_contract.empty())
				contracts.emplace_back("*");
			for (auto const& contract: contracts)
				if (
					_outputSelection[file].find(contract) != _outputSelection.end() &&
					_outputSelection[file][contract].is_array() &&
					isArtifactRequested(_outputSelection[file][contract], _artifact)
				)
					return true;
		}

	return false;
}

bool isArtifactRequested(Json const& _outputSelection, string const& _file, string const& _contract, vector<string> const& _artifacts)
{
	for (auto const& artifact: _artifacts)
		if (isArtifactRequested(_outputSelection, _file, _contract, artifact))
			return true;
	return false;
}

Json formatLinkReferences(std::map<size_t, std::string> const& linkReferences)
{
	Json ret(Json::object());

	for (auto const& ref: linkReferences)
	{
		string const& fullname = ref.second;
		size_t colon = fullname.rfind(':');
		solAssert(colon != string::npos, "");
		string file = fullname.substr(0, colon);
		string name = fullname.substr(colon + 1);

		Json fileObject = ret.value(file, Json::object());
		Json libraryArray = fileObject.value(name, Json::array());

		Json entry = Json::object();
		entry["start"] = Json(ref.first);
		entry["length"] = 20;

		libraryArray.emplace_back(entry);
		fileObject[name] = libraryArray;
		ret[file] = fileObject;
	}

	return ret;
}

Json collectEVMObject(eth::LinkerObject const& _object, string const* _sourceMap)
{
	Json output = Json::object();
	output["object"] = _object.toHex();
	output["opcodes"] = solidity::disassemble(_object.bytecode);
	output["sourceMap"] = _sourceMap ? *_sourceMap : "";
	output["linkReferences"] = formatLinkReferences(_object.linkReferences);
	return output;
}

}

Json StandardCompiler::compileInternal(Json const& _input)
{
	m_compilerStack.reset(false);

	if (!_input.is_object())
		return formatFatalError("JSONError", "Input is not a JSON object.");

	if (_input["language"] != "Solidity")
		return formatFatalError("JSONError", "Only \"Solidity\" is supported as a language.");

	Json const& sources = _input["sources"];

	if (!sources.is_object() && !sources.is_null())
		return formatFatalError("JSONError", "\"sources\" is not a JSON object.");

	if (sources.empty())
		return formatFatalError("JSONError", "No input sources specified.");

	Json errors = Json::array();

	for (auto const& sourceName: sources.items())
	{
		string hash;

		if (!sources[sourceName.key()].is_object())
			return formatFatalError("JSONError", "Source input is not a JSON object.");

		if (sources[sourceName.key()]["keccak256"].is_string())
			hash = sources[sourceName.key()]["keccak256"].get<string>();

		if (sources[sourceName.key()]["content"].is_string())
		{
			string content = sources[sourceName.key()]["content"].get<string>();
			if (!hash.empty() && !hashMatchesContent(hash, content))
				errors.emplace_back(formatError(
					false,
					"IOError",
					"general",
					"Mismatch between content and supplied hash for \"" + sourceName.key() + "\""
				));
			else
				m_compilerStack.addSource(sourceName.key(), content);
		}
		else if (sources[sourceName.key()]["urls"].is_array())
		{
			if (!m_readFile)
				return formatFatalError("JSONError", "No import callback supplied, but URL is requested.");

			bool found = false;
			vector<string> failures;

			for (auto const& url: sources[sourceName.key()]["urls"])
			{
				ReadCallback::Result result = m_readFile(url.get<string>());
				if (result.success)
				{
					if (!hash.empty() && !hashMatchesContent(hash, result.responseOrErrorMessage))
						errors.emplace_back(formatError(
							false,
							"IOError",
							"general",
							"Mismatch between content and supplied hash for \"" + sourceName.key() + "\" at \"" + url.get<string>() + "\""
						));
					else
					{
						m_compilerStack.addSource(sourceName.key(), result.responseOrErrorMessage);
						found = true;
						break;
					}
				}
				else
					failures.push_back("Cannot import url (\"" + url.get<string>() + "\"): " + result.responseOrErrorMessage);
			}

			for (auto const& failure: failures)
			{
				/// If the import succeeded, let mark all the others as warnings, otherwise all of them are errors.
				errors.emplace_back(formatError(
					found ? true : false,
					"IOError",
					"general",
					failure
				));
			}
		}
		else
			return formatFatalError("JSONError", "Invalid input source specified.");
	}

	Json const& settings = _input.value("settings", Json());

	if (settings.find("evmVersion") != settings.end())
	{
		boost::optional<EVMVersion> version = EVMVersion::fromString(settings.value("evmVersion", {}));
		if (!version)
			return formatFatalError("JSONError", "Invalid EVM version requested.");
		m_compilerStack.setEVMVersion(*version);
	}

	vector<string> remappings;
	for (auto const& remapping: settings.value("remappings", Json()))
		remappings.push_back(remapping.get<string>());
	m_compilerStack.setRemappings(remappings);

	Json optimizerSettings = settings.value("optimizer", Json());
	bool const optimize = optimizerSettings.value("enabled", false);
	unsigned const optimizeRuns = optimizerSettings.value("runs", 200u);
	m_compilerStack.setOptimiserSettings(optimize, optimizeRuns);

	map<string, h160> libraries;
	Json jsonLibraries = settings.value("libraries", Json(Json::object()));
	if (!jsonLibraries.is_object())
		return formatFatalError("JSONError", "\"libraries\" is not a JSON object.");
	for (auto const& sourceName: jsonLibraries.items())
	{
		auto const& jsonSourceName = jsonLibraries[sourceName.key()];
		if (!jsonSourceName.is_object())
			return formatFatalError("JSONError", "library entry is not a JSON object.");
		for (auto const& library: jsonSourceName.items())
		{
			string address = jsonSourceName[library.key()].get<string>();

			if (!boost::starts_with(address, "0x"))
				return formatFatalError(
					"JSONError",
					"Library address is not prefixed with \"0x\"."
				);

			if (address.length() != 42)
				return formatFatalError(
					"JSONError",
					"Library address is of invalid length."
				);

			try
			{
				// @TODO use libraries only for the given source
				libraries[library.key()] = h160(address);
			}
			catch (dev::BadHexCharacter const&)
			{
				return formatFatalError(
					"JSONError",
					"Invalid library address (\"" + address + "\") supplied."
				);
			}
		}
	}
	m_compilerStack.setLibraries(libraries);

	Json metadataSettings = settings.value("metadata", Json());
	m_compilerStack.useMetadataLiteralSources(metadataSettings.value("useLiteralContent", Json(false)).get<bool>());

	Json outputSelection = settings.value("outputSelection", Json());
	m_compilerStack.setRequestedContractNames(requestedContractNames(outputSelection));

	auto scannerFromSourceName = [&](string const& _sourceName) -> solidity::Scanner const& { return m_compilerStack.scanner(_sourceName); };

	try
	{
		m_compilerStack.compile();

		for (auto const& error: m_compilerStack.errors())
		{
			Error const& err = dynamic_cast<Error const&>(*error);

			errors.emplace_back(formatErrorWithException(
				*error,
				err.type() == Error::Type::Warning,
				err.typeName(),
				"general",
				"",
				scannerFromSourceName
			));
		}
	}
	/// This is only thrown in a very few locations.
	catch (Error const& _error)
	{
		errors.emplace_back(formatErrorWithException(
			_error,
			false,
			_error.typeName(),
			"general",
			"Uncaught error: ",
			scannerFromSourceName
		));
	}
	/// This should not be leaked from compile().
	catch (FatalError const& _exception)
	{
		errors.emplace_back(formatError(
			false,
			"FatalError",
			"general",
			"Uncaught fatal error: " + boost::diagnostic_information(_exception)
		));
	}
	catch (CompilerError const& _exception)
	{
		errors.emplace_back(formatErrorWithException(
			_exception,
			false,
			"CompilerError",
			"general",
			"Compiler error (" + _exception.lineInfo() + ")",
			scannerFromSourceName
		));
	}
	catch (InternalCompilerError const& _exception)
	{
		errors.emplace_back(formatErrorWithException(
			_exception,
			false,
			"InternalCompilerError",
			"general",
			"Internal compiler error (" + _exception.lineInfo() + ")",
			scannerFromSourceName
		));
	}
	catch (UnimplementedFeatureError const& _exception)
	{
		errors.emplace_back(formatErrorWithException(
			_exception,
			false,
			"UnimplementedFeatureError",
			"general",
			"Unimplemented feature (" + _exception.lineInfo() + ")",
			scannerFromSourceName
		));
	}
	catch (Exception const& _exception)
	{
		errors.emplace_back(formatError(
			false,
			"Exception",
			"general",
			"Exception during compilation: " + boost::diagnostic_information(_exception)
		));
	}
	catch (...)
	{
		errors.emplace_back(formatError(
			false,
			"Exception",
			"general",
			"Unknown exception during compilation."
		));
	}

	bool const analysisSuccess = m_compilerStack.state() >= CompilerStack::State::AnalysisSuccessful;
	bool const compilationSuccess = m_compilerStack.state() == CompilerStack::State::CompilationSuccessful;

	/// Inconsistent state - stop here to receive error reports from users
	if (!compilationSuccess && errors.empty())
		return formatFatalError("InternalCompilerError", "No error reported, but compilation failed.");

	Json output = Json::object();

	if (!errors.empty())
		output["errors"] = errors;

	output["sources"] = Json::object();
	unsigned sourceIndex = 0;
	for (string const& sourceName: analysisSuccess ? m_compilerStack.sourceNames() : vector<string>())
	{
		Json sourceResult = Json::object();
		sourceResult["id"] = sourceIndex++;
		if (isArtifactRequested(outputSelection, sourceName, "", "ast"))
			sourceResult["ast"] = ASTJsonConverter(false, m_compilerStack.sourceIndices()).toJson(m_compilerStack.ast(sourceName));
		if (isArtifactRequested(outputSelection, sourceName, "", "legacyAST"))
			sourceResult["legacyAST"] = ASTJsonConverter(true, m_compilerStack.sourceIndices()).toJson(m_compilerStack.ast(sourceName));
		output["sources"][sourceName] = sourceResult;
	}

	Json contractsOutput = Json::object();
	for (string const& contractName: compilationSuccess ? m_compilerStack.contractNames() : vector<string>())
	{
		size_t colon = contractName.rfind(':');
		solAssert(colon != string::npos, "");
		string file = contractName.substr(0, colon);
		string name = contractName.substr(colon + 1);

		// ABI, documentation and metadata
		Json contractData(Json::object());
		if (isArtifactRequested(outputSelection, file, name, "abi"))
			contractData["abi"] = m_compilerStack.contractABI(contractName);
		if (isArtifactRequested(outputSelection, file, name, "metadata"))
			contractData["metadata"] = m_compilerStack.metadata(contractName);
		if (isArtifactRequested(outputSelection, file, name, "userdoc"))
			contractData["userdoc"] = m_compilerStack.natspecUser(contractName);
		if (isArtifactRequested(outputSelection, file, name, "devdoc"))
			contractData["devdoc"] = m_compilerStack.natspecDev(contractName);

		// EVM
		Json evmData(Json::object());
		// @TODO: add ir
		if (isArtifactRequested(outputSelection, file, name, "evm.assembly"))
			evmData["assembly"] = m_compilerStack.assemblyString(contractName, createSourceList(_input));
		if (isArtifactRequested(outputSelection, file, name, "evm.legacyAssembly"))
			evmData["legacyAssembly"] = m_compilerStack.assemblyJSON(contractName, createSourceList(_input));
		if (isArtifactRequested(outputSelection, file, name, "evm.methodIdentifiers"))
			evmData["methodIdentifiers"] = m_compilerStack.methodIdentifiers(contractName);
		if (isArtifactRequested(outputSelection, file, name, "evm.gasEstimates"))
			evmData["gasEstimates"] = m_compilerStack.gasEstimates(contractName);

		if (isArtifactRequested(
			outputSelection,
			file,
			name,
			{ "evm.bytecode", "evm.bytecode.object", "evm.bytecode.opcodes", "evm.bytecode.sourceMap", "evm.bytecode.linkReferences" }
		))
			evmData["bytecode"] = collectEVMObject(
				m_compilerStack.object(contractName),
				m_compilerStack.sourceMapping(contractName)
			);

		if (isArtifactRequested(
			outputSelection,
			file,
			name,
			{ "evm.deployedBytecode", "evm.deployedBytecode.object", "evm.deployedBytecode.opcodes", "evm.deployedBytecode.sourceMap", "evm.deployedBytecode.linkReferences" }
		))
			evmData["deployedBytecode"] = collectEVMObject(
				m_compilerStack.runtimeObject(contractName),
				m_compilerStack.runtimeSourceMapping(contractName)
			);

		contractData["evm"] = evmData;

		if (contractsOutput.find(file) == contractsOutput.end())
			contractsOutput[file] = Json::object();

		contractsOutput[file][name] = contractData;
	}
	output["contracts"] = contractsOutput;

	return output;
}

Json StandardCompiler::compile(Json const& _input)
{
	try
	{
		return compileInternal(_input);
	}
	catch (Json::exception const& _exception)
	{
		return formatFatalError("InternalCompilerError", string("JSON exception: ") + _exception.what());
	}
	catch (Exception const& _exception)
	{
		return formatFatalError("InternalCompilerError", "Internal exception in StandardCompiler::compileInternal: " + boost::diagnostic_information(_exception));
	}
	catch (...)
	{
		return formatFatalError("InternalCompilerError", "Internal exception in StandardCompiler::compileInternal");
	}
}

string StandardCompiler::compile(string const& _input)
{
	Json input;
	string errors;
	try
	{
		if (!jsonParseStrict(_input, input, &errors))
			return jsonCompactPrint(formatFatalError("JSONError", errors));
	}
	catch(...)
	{
		return "{\"errors\":\"[{\"type\":\"JSONError\",\"component\":\"general\",\"severity\":\"error\",\"message\":\"Error parsing input JSON.\"}]}";
	}

	// cout << "Input: " << input.toStyledString() << endl;
	Json output = compile(input);
	// cout << "Output: " << output.toStyledString() << endl;

	try
	{
		return jsonCompactPrint(output);
	}
	catch(...)
	{
		return "{\"errors\":\"[{\"type\":\"JSONError\",\"component\":\"general\",\"severity\":\"error\",\"message\":\"Error writing output JSON.\"}]}";
	}
}
