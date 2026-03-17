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

#include "hex_data_formatter.hpp"

#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "sending/constants.hpp"

namespace Sending {

HexDataFormatter::HexDataFormatter(QLineEdit* lineEdit, const int maxBytes, QObject* parent)
    : QObject(parent ? parent : lineEdit),
      m_lineEdit(lineEdit),
      m_maxBytes(maxBytes),
      m_isFormatting(false)
{
    // Setup hex checkk and length constraint
    m_lineEdit->setValidator(
        new QRegularExpressionValidator(Constants::HEX_VALIDATION_PATTERN, m_lineEdit));
    const int maxLength = maxBytes * 2 + (maxBytes - 1);
    m_lineEdit->setMaxLength(maxLength);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &HexDataFormatter::onTextChanged);
}

void HexDataFormatter::setMaxBytes(const int maxBytes)
{
    m_maxBytes = maxBytes;
    const int maxLength = maxBytes * 2 + (maxBytes - 1);
    m_lineEdit->setMaxLength(maxLength);
}

void HexDataFormatter::onTextChanged(const QString& text)
{
    if (m_isFormatting) return;
    m_isFormatting = true;

    // Extract raw hex digits only
    QString clean;
    clean.reserve(m_maxBytes * 2);
    int hexCharsBeforeCursor = 0;
    const int oldCursorPos = m_lineEdit->cursorPosition();

    for (int i = 0; i < text.length(); ++i)
    {
        if (text[i].isDigit() || (text[i] >= 'A' && text[i] <= 'F') ||
            (text[i] >= 'a' && text[i] <= 'f'))
        {
            if (clean.length() < m_maxBytes * Constants::HEX_CHARS_PER_BYTE)
            {
                clean.append(text[i].toUpper());
                if (i < oldCursorPos) hexCharsBeforeCursor++;
            }
        }
    }

    // Build formatted string and update the widget
    QString formatted;
    formatted.reserve(clean.length() + (clean.length() / 2));
    for (int i = 0; i < clean.length(); ++i)
    {
        if (i > 0 && i % Constants::HEX_CHARS_PER_BYTE == 0)
            formatted.append(Constants::BYTE_SEPARATOR);
        formatted.append(clean[i]);
    }
    m_lineEdit->setText(formatted);

    // Calculate Cursor
    int newCursorPos =
        hexCharsBeforeCursor + (hexCharsBeforeCursor > 0 ? (hexCharsBeforeCursor - 1) / 2 : 0);
    if (hexCharsBeforeCursor % 2 == 0 && hexCharsBeforeCursor > 0 &&
        newCursorPos < formatted.length())
    {
        newCursorPos++;
    }

    m_lineEdit->setCursorPosition(qMin(newCursorPos, formatted.length()));
    m_isFormatting = false;
}

}  // namespace Sending
