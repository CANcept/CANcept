//
// Created by flori on 05.01.2026.
//

#ifndef CANBUSMANAGER_FILE_PARSER_HPP
#define CANBUSMANAGER_FILE_PARSER_HPP
#include <string>
namespace CanHandler {
class FileParser
{
   public:
    auto parseFile(std::string filePath) -> std::string*;
};
}  // namespace CanHandler
#endif  // CANBUSMANAGER_FILE_PARSER_HPP
