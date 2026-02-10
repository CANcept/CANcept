#pragma once
#include <string>
namespace CanHandler {
class FileParser
{
   public:
    static auto parseFile(const std::string &filePath) -> std::string *;
};
}  // namespace CanHandler
