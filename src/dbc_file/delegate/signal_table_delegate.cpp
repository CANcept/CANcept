#include "signal_table_delegate.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "core/macro/theme.hpp"
#include <QPainter>

namespace DbcFile {

SignalTableDelegate::SignalTableDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void SignalTableDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // Paint background
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->fillRect(opt.rect, colors.surfaceMain);

    // Grid line
    painter->setPen(QPen(QColor(colors.borderSubtle.name(QColor::HexArgb)), 0));
    painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());

    // Font
    painter->setPen(colors.textPrimary);
    QFont font = opt.font;
    font.setPixelSize(spacing.fontSizeSm);
    painter->setFont(font);

    // Padding Rect
    QRect paddedRect = opt.rect.adjusted(spacing.spacingSm, 0, -spacing.spacingSm, 0);

    // --- COLUMN LOGIC ---

    // Signal Name
    if (index.column() == Constants::Columns::SigName)
    {
        painter->setFont(font);
        painter->drawText(paddedRect, Qt::AlignVCenter | Qt::AlignCenter, opt.text);
    }
    // Message ID Badge + Name
    else if (index.column() == Constants::Columns::SigMessage)
    {
        // get ID From model
        uint msgId = index.data(DbcRoles::Role_Id).toUInt();
        QString idText = QString("0x%1").arg(msgId, 3, 16, QChar('0')).toUpper();
        QString msgName = index.data(Qt::DisplayRole).toString();

        // draw badge
        QFont badgeFont = font;
        badgeFont.setPixelSize(spacing.fontSizeXs);
        QFontMetrics fm(badgeFont);
        int badgeWidth = fm.horizontalAdvance(idText) + 12;
        int badgeHeight = 18;

        QRect badgeRect(paddedRect.left(), paddedRect.center().y() - badgeHeight / 2, badgeWidth, badgeHeight);

        // Badge Background
        painter->setBrush(colors.surfaceMain);
        painter->setPen(QPen(colors.borderSubtle, 1));
        painter->drawRoundedRect(badgeRect, spacing.radiusSm / 2, spacing.radiusSm / 2);

        // Badge Text
        painter->setPen(colors.textSecondary);
        painter->setFont(badgeFont);
        painter->drawText(badgeRect, Qt::AlignCenter, idText);

        // Name
        painter->setFont(font); // Reset
        painter->setPen(colors.textPrimary);
        QRect textRect = paddedRect.adjusted(badgeWidth + 10, 0, 0, 0);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignCenter, msgName);
    }
    // Range: [min, max]
    else if (index.column() == Constants::Columns::SigMin)
    {
        double min = index.data(Qt::DisplayRole).toDouble();
        double max = index.sibling(index.row(), Constants::Columns::SigMax).data().toDouble();
        QString text = QString("[%1, %2]").arg(min).arg(max);
        painter->drawText(paddedRect, Qt::AlignVCenter | Qt::AlignCenter, text);
    }
    // Length: Value + "Bit"
    else if (index.column() == Constants::Columns::SigLength)
    {
        QString val = index.data(Qt::DisplayRole).toString();
        if (!val.isEmpty()) val += Constants::SignalsPage::LengthUnit;
        painter->drawText(paddedRect, Qt::AlignVCenter | Qt::AlignCenter, val);
    }
    else if (index.column() == Constants::Columns::SigUnit)
    {
        QString val = Constants::SignalsPage::DefaultUnit;
        if (!index.data(Role_Unit).toString().isEmpty())
        {
            val = index.data(Role_Unit).toString();
        }
        painter->drawText(paddedRect, Qt::AlignVCenter, val);
    }
    // Standard Columns (Factor, Offset, ByteOrder, Type, StartBit)
    else
    {
        QString text = opt.text;
        if (text.isEmpty()) {
            text = index.data(Qt::DisplayRole).toString();
        }
        painter->drawText(paddedRect, Qt::AlignVCenter | Qt::AlignCenter, text);
    }

    painter->restore();
}

auto SignalTableDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const -> QSize
{
    return {option.rect.width(), THEME.spacing().HeightSm};
}

} // namespace DbcFile