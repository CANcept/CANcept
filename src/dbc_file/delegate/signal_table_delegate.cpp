#include "signal_table_delegate.hpp"

#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace DbcFile {

SignalTableDelegate::SignalTableDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void SignalTableDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // Paint row background and separation line
    Core::ItemPainter::paintRow(painter, option.rect, false, false);

    // Padding Rect for content
    const int padding = spacing.spacingSm;
    QRect cellRect = option.rect.adjusted(padding, 0, -padding, 0);

    // --- Column Logic ---

    // 1. Message Column (Badge + Text)
    if (index.column() == Constants::Columns::SigMessage)
    {
        uint msgId = index.data(DbcRoles::Role_Id).toUInt();
        QString idText = QString("0x%1").arg(msgId, 3, 16, QChar('0')).toUpper();
        QString msgName = index.data(Qt::DisplayRole).toString();

        QSize badgeSize = Core::ItemPainter::measureBadge(idText);
        QRect badgeRect(cellRect.left(), cellRect.center().y() - badgeSize.height() / 2 + 1,
                        badgeSize.width(), badgeSize.height());

        // Custom ID badge style
        Core::ItemPainter::BadgeStyle badgeStyle;
        badgeStyle.background = Qt::transparent;
        badgeStyle.text = colors.textSecondary;
        badgeStyle.border = colors.borderSubtle;
        Core::ItemPainter::paintBadge(painter, badgeRect, idText, QIcon(), &badgeStyle);

        // Paint text next to badge
        int textOffset = badgeSize.width() + padding;
        QRect textRect = cellRect.adjusted(textOffset, 0, 0, 0);

        Core::ItemPainter::paintText(painter, textRect, msgName);
    }
    // 2. Range Column
    else if (index.column() == Constants::Columns::SigMin)
    {
        double min = index.data(Qt::DisplayRole).toDouble();
        double max = index.sibling(index.row(), Constants::Columns::SigMax).data().toDouble();
        QString text = QString("[%1, %2]").arg(min).arg(max);

        Core::ItemPainter::paintText(painter, cellRect, text, false, QColor(), Qt::AlignCenter);
    }
    // 3. Length Column
    else if (index.column() == Constants::Columns::SigLength)
    {
        QString val = index.data(Qt::DisplayRole).toString();
        if (!val.isEmpty()) val += Constants::SignalsPage::LengthUnit;
        Core::ItemPainter::paintText(painter, cellRect, val, false, QColor(), Qt::AlignCenter);
    }
    // 4. Unit Column
    else if (index.column() == Constants::Columns::SigUnit)
    {
        QString val = Constants::SignalsPage::DefaultUnit;
        if (!index.data(Role_Unit).toString().isEmpty()) val = index.data(Role_Unit).toString();
        Core::ItemPainter::paintText(painter, cellRect, val, false, QColor(), Qt::AlignCenter);
    }
    // 5. Standard columns
    else
    {
        QString text = index.data(Qt::DisplayRole).toString();
        bool isBold = (index.column() == Constants::Columns::SigName);

        Core::ItemPainter::paintText(painter, cellRect, text, isBold, QColor(), Qt::AlignCenter);
    }
}

auto SignalTableDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const -> QSize
{
    const auto& spacing = THEME.spacing();
    return {option.rect.width(), spacing.HeightSm};
}

}  // namespace DbcFile