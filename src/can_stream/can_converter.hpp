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

#pragma once

#include <string>

#include "can_stream/export_type.hpp"

namespace CanStream {

/**
 * @brief Converts a CAN log file from MDF4 to a requested output format.
 *
 * Internally opens an Mdf4Reader on the source file and constructs the
 * corresponding writer.
 */
class CanConverter
{
   public:
    CanConverter(std::string sourcePath, std::string targetPath, ExportType format);

    /** @brief Runs the conversion. Returns the output path on success, empty on failure. */
    [[nodiscard]] auto convert() -> std::string;

   private:
    std::string m_source;
    std::string m_target;
    ExportType m_format;
};

}  // namespace CanStream