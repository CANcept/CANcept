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

#include <QLineEdit>
#include <QObject>

#include "sending/constants.hpp"

namespace Sending {

/**
 * @class HexDataFormatter
 * @brief Formats hexadecimal data input with automatic byte spacing.
 ** Checks:
 * - Automatically inserts spaces between bytes (every 2 hex chars)
 * - Restricts input to maximum number of bytes
 * - Only allows hex characters
 */
class HexDataFormatter final : public QObject
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a hex data formatter.
     * @param lineEdit The widget to format
     * @param maxBytes Maximum number of byts allowed
     * @param parent Parent QObject
     */
    explicit HexDataFormatter(QLineEdit* lineEdit, int maxBytes = Constants::MAX_CAN_DLC,
                              QObject* parent = nullptr);

    /**
     * @brief Sets the maximum number of bytes.
     * @param maxBytes Maximum number of bytes allowed
     */
    void setMaxBytes(int maxBytes);

   private slots:
    void onTextChanged(const QString& text);

   private:
    QLineEdit* m_lineEdit;
    int m_maxBytes;
    bool m_isFormatting;
};

}  // namespace Sending
