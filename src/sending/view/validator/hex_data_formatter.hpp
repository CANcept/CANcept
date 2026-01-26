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
     * @param lineEdit The QLineEdit widget to format
     * @param maxBytes Maximum number of bytes allowed
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
