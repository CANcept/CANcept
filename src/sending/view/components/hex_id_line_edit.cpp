#include "hex_id_line_edit.hpp"

#include <QPaintEvent>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"
#include "sending/view/validator/hex_validator.hpp"

namespace Sending {

HexIdLineEdit::HexIdLineEdit(QWidget* parent) : StyledLineEdit(parent)
{
    updateTextMargins();
    setPlaceholderText(Constants::CAN_ID_PLACEHOLDER);
    setMaxLength(Constants::MAX_CAN_ID_HEX_LENGTH);

    const auto* validator = new HexValidator(Constants::MIN_CAN_ID, Constants::MAX_CAN_ID, this);
    setValidator(validator);
}

void HexIdLineEdit::setMaxHexValue(uint32_t maxValue)
{
    const auto* validator = new HexValidator(Constants::MIN_CAN_ID, maxValue, this);
    setValidator(validator);
}

void HexIdLineEdit::paintEvent(QPaintEvent* event)
{
    StyledLineEdit::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    QColor prefixColor = colors.textSecondary;
    if (hasFocus())
    {
        prefixColor = colors.textPrimary;
    }

    // Set font
    const QFont prefixFont = font();
    painter.setFont(prefixFont);
    painter.setPen(prefixColor);

    // Draw decoration
    const QRect prefixRect(spacing.spacingXl, 0, Constants::HEX_PREFIX_DISPLAY_WIDTH, height());
    painter.drawText(prefixRect, Qt::AlignLeft | Qt::AlignVCenter, Constants::HEX_PREFIX);
}

void HexIdLineEdit::updateTextMargins()
{
    const auto& spacing = THEME.spacing();
    // Add left margin for decoration
    setTextMargins(Constants::HEX_PREFIX_DISPLAY_WIDTH + spacing.spacingMd, 0, 0, 0);
}

}  // namespace Sending
