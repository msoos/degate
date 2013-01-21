/*

 This file is part of the IC reverse engineering tool degate.

 Copyright 2008, 2009, 2010 by Martin Schobert

 Degate is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.

 Degate is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with degate. If not, see <http://www.gnu.org/licenses/>.

*/

#include "DOTAttributes.h"

using namespace degate;

void DOTAttributes::add(
    std::string const& attribute_name,
    std::string const& value
) {
    std::ostringstream stm;
    stm << attribute_name << "=\"" << value << "\"";
    attributes.push_back(std::make_pair(attribute_name, stm.str()));
}


std::string DOTAttributes::get_string() const {
    std::string result;

    for(std::vector<std::pair<std::string, std::string> >::const_iterator
        iter = attributes.begin()
        ; iter != attributes.end()
        ; ++iter
    ) {
        if (!result.empty()) {
            result += ", ";
        }
        result += iter->second;
    }

    return std::string("[") + result + std::string("];");
}

