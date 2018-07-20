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
/** @file JSON.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include "JSON.h"

#include <sstream>
#include <map>
#include <memory>

using namespace std;

namespace dev {

string jsonPrettyPrint(Json const &_input)
{
	return _input.dump(4);
}

string jsonCompactPrint(Json const &_input)
{
	return _input.dump(0);
}

bool jsonParseStrict(string const &_input, Json &_json, string *_errs /* = nullptr */)
{
	try {
		// parsing input with a syntax error
		_json = Json::parse(_input);
		return true;
	}
	catch (Json::parse_error &e) {
		/*
		std::cout << "message: " << e.what() << '\n'
				  << "exception id: " << e.id << '\n'
				  << "byte position of error: " << e.byte << std::endl;
		*/
		if (_errs) {
			*_errs = e.what();
		}
		return false;
	}
}

bool jsonParse(string const &_input, Json &_json, string *_errs /* = nullptr */)
{
	try {
		// parsing input with a syntax error
		_json = Json::parse(_input);
		return true;
	}
	catch (Json::parse_error &e) {
		/*
		std::cout << "message: " << e.what() << '\n'
				  << "exception id: " << e.id << '\n'
				  << "byte position of error: " << e.byte << std::endl;
		*/
		if (_errs) {
			*_errs = e.what();
		}
		return false;
	}
}

} // namespace dev
