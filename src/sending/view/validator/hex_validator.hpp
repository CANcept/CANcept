#pragma once

#include <QValidator>
#include <optional>

namespace Sending {

/**
 * @class HexValidator
 * @brief Generic hexadecimal input validator with optional range constraints.
 * - Validates hex characters (0-9, A-F, case-insensitive)
 * - Optional minimum value constraint
 * - Optional maximum value constraint
 */
class HexValidator final : public QValidator
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a hex validator with optional range constraints.
     * @param minValue Minimum allowed value
     * @param maxValue Maximum allowed value
     * @param parent Parent QObject
     */
    explicit HexValidator(std::optional<uint64_t> minValue = std::nullopt,
                          std::optional<uint64_t> maxValue = std::nullopt,
                          QObject* parent = nullptr);

    /**
     * @brief Validates the input string.
     * @param input The text to validate (will be converted to uppercase)
     * @param pos Cursor position (unused but required by Qt)
     * @return Acceptable if valid hex within range, Invalid otherwise
     */
    State validate(QString& input, int& pos) const override;

    /**
     * @brief Sets the maximum value constraint.
     * @param maxValue Maximum allowed value
     */
    void setMaxValue(uint64_t maxValue);

    /**
     * @brief Sets the minimum value constraint.
     * @param minValue Minimum allowed value
     */
    void setMinValue(uint64_t minValue);

   private:
    std::optional<uint64_t> m_minValue;
    std::optional<uint64_t> m_maxValue;
};

}  // namespace Sending
