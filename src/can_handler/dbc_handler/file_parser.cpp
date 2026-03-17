/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by flori on 21.01.2026.
//
#include "file_parser.hpp"

#include <fstream>
namespace CanHandler {
auto FileParser::parseFile(const std::string& filePath) -> std::string*
{
    std::ifstream dbcFile(filePath);
    if (!dbcFile.is_open())
    {
        return nullptr;
    }
    std::string* dbcString = new std::string();
    std::string line;
    while (std::getline(dbcFile, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        dbcString->append(line + " ");
    }
    dbcFile.close();
    return dbcString;
}

}  // namespace CanHandler