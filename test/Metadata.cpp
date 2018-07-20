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
 * @date 2017
 * Metadata processing helpers.
 */

#include <string>
#include <iostream>
#include <libdevcore/JSON.h>

using namespace std;

namespace dev
{
namespace test
{

string bytecodeSansMetadata(string const& _bytecode)
{
	/// The metadata hash takes up 43 bytes (or 86 characters in hex)
	/// /a165627a7a72305820([0-9a-f]{64})0029$/

	if (_bytecode.size() < 88)
		return _bytecode;

	if (_bytecode.substr(_bytecode.size() - 4, 4) != "0029")
		return _bytecode;

	if (_bytecode.substr(_bytecode.size() - 86, 18) != "a165627a7a72305820")
		return _bytecode;

	return _bytecode.substr(0, _bytecode.size() - 86);
}

bool isValidMetadata(string const& _metadata)
{
	Json metadata;
	if (!jsonParseStrict(_metadata, metadata))
		return false;

	if (
		!metadata.is_object() ||
		metadata.find("version") == metadata.end() ||
		metadata.find("language") == metadata.end() ||
		metadata.find("compiler") == metadata.end() ||
		metadata.find("settings") == metadata.end() ||
		metadata.find("sources") == metadata.end() ||
		metadata.find("output") == metadata.end() ||
		metadata["settings"].find("evmVersion") == metadata["settings"].end()
	)
		return false;

	if (!metadata["version"].is_number() || metadata["version"] != 1)
		return false;

	if (!metadata["language"].is_string() || metadata["language"].get<string>() != "Solidity")
		return false;

	/// @TODO add more strict checks

	return true;
}

}
} // end namespaces
