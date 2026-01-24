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
        dbcString->append(line + " ");
    }
    dbcFile.close();
    return dbcString;
}

}  // namespace CanHandler