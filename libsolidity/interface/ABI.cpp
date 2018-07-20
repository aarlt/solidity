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
 * Utilities to handle the Contract ABI (https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI)
 */

#include <libsolidity/interface/ABI.h>
#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Json ABI::generate(ContractDefinition const& _contractDef)
{
	Json abi(Json::array());

	for (auto it: _contractDef.interfaceFunctions())
	{
		auto externalFunctionType = it.second->interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json method;
		method["type"] = "function";
		method["name"] = it.second->declaration().name();
		// TODO: deprecate constant in a future release
		method["constant"] = externalFunctionType->stateMutability() == StateMutability::Pure || it.second->stateMutability() == StateMutability::View;
		method["payable"] = externalFunctionType->isPayable();
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		method["inputs"] = formatTypeList(
			externalFunctionType->parameterNames(),
			externalFunctionType->parameterTypes(),
			_contractDef.isLibrary()
		);
		method["outputs"] = formatTypeList(
			externalFunctionType->returnParameterNames(),
			externalFunctionType->returnParameterTypes(),
			_contractDef.isLibrary()
		);
		abi.emplace_back(method);
	}
	if (_contractDef.constructor())
	{
		auto externalFunctionType = FunctionType(*_contractDef.constructor(), false).interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json method;
		method["type"] = "constructor";
		method["payable"] = externalFunctionType->isPayable();
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		method["inputs"] = formatTypeList(
			externalFunctionType->parameterNames(),
			externalFunctionType->parameterTypes(),
			_contractDef.isLibrary()
		);
		abi.emplace_back(method);
	}
	if (_contractDef.fallbackFunction())
	{
		auto externalFunctionType = FunctionType(*_contractDef.fallbackFunction(), false).interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json method;
		method["type"] = "fallback";
		method["payable"] = externalFunctionType->isPayable();
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		abi.emplace_back(method);
	}
	for (auto const& it: _contractDef.interfaceEvents())
	{
		Json event;
		event["type"] = "event";
		event["name"] = it->name();
		event["anonymous"] = it->isAnonymous();
		Json params(Json::array());
		for (auto const& p: it->parameters())
		{
			auto type = p->annotation().type->interfaceType(false);
			solAssert(type, "");
			Json input;
			auto param = formatType(p->name(), *type, false);
			param["indexed"] = p->isIndexed();
			params.emplace_back(param);
		}
		event["inputs"] = params;
		abi.emplace_back(event);
	}

	return abi;
}

Json ABI::formatTypeList(
	vector<string> const& _names,
	vector<TypePointer> const& _types,
	bool _forLibrary
)
{
	Json params(Json::array());
	solAssert(_names.size() == _types.size(), "Names and types vector size does not match");
	for (unsigned i = 0; i < _names.size(); ++i)
	{
		solAssert(_types[i], "");
		params.emplace_back(formatType(_names[i], *_types[i], _forLibrary));
	}
	return params;
}

Json ABI::formatType(string const& _name, Type const& _type, bool _forLibrary)
{
	Json ret;
	ret["name"] = _name;
	string suffix = (_forLibrary && _type.dataStoredIn(DataLocation::Storage)) ? " storage" : "";
	if (_type.isValueType() || (_forLibrary && _type.dataStoredIn(DataLocation::Storage)))
		ret["type"] = _type.canonicalName() + suffix;
	else if (ArrayType const* arrayType = dynamic_cast<ArrayType const*>(&_type))
	{
		if (arrayType->isByteArray())
			ret["type"] = _type.canonicalName() + suffix;
		else
		{
			string suffix;
			if (arrayType->isDynamicallySized())
				suffix = "[]";
			else
				suffix = string("[") + arrayType->length().str() + "]";
			solAssert(arrayType->baseType(), "");
			Json subtype = formatType("", *arrayType->baseType(), _forLibrary);
			if (subtype.find("components") != subtype.end())
			{
				ret["type"] = subtype["type"].get<string>() + suffix;
				ret["components"] = subtype["components"];
			}
			else
				ret["type"] = subtype["type"].get<string>() + suffix;
		}
	}
	else if (StructType const* structType = dynamic_cast<StructType const*>(&_type))
	{
		ret["type"] = "tuple";
		ret["components"] = Json::array();
		for (auto const& member: structType->members(nullptr))
		{
			solAssert(member.type, "");
			auto t = member.type->interfaceType(_forLibrary);
			solAssert(t, "");
			ret["components"].emplace_back(formatType(member.name, *t, _forLibrary));
		}
	}
	else
		solAssert(false, "Invalid type.");
	return ret;
}
