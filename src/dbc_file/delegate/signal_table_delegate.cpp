#include "signal_table_delegate.hpp"

#include "core/util/dbc_utils.hpp"
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

    // Message Column (Badge + Text)
    if (index.column() == Constants::Columns::SigMessage)
    {
        uint msgId = index.data(DbcRoles::Role_Id).toUInt();
        QString idText = Core::formatId(msgId);
        QString msgName = index.data(Qt::DisplayRole).toString();

        // --- Measure badge ---
        QSize badgeSize = Core::ItemPainter::measureBadge(idText);

        // --- Measure text ---
        painter->save();
        QFontMetrics fm(painter->font());
        int textWidth = fm.horizontalAdvance(msgName);
        int textHeight = fm.height();
        painter->restore();

        const int spacingBetween = padding;  // distance between badge and text

        // --- Total width of (badge + spacing + text) ---
        int totalWidth = badgeSize.width() + spacingBetween + textWidth;

        // --- Calculate centered start X ---
        int startX = cellRect.left() + (cellRect.width() - totalWidth) / 2;
        int centerY = cellRect.center().y();

        // --- Badge rect ---
        QRect badgeRect(startX, centerY - badgeSize.height() / 2, badgeSize.width(),
                        badgeSize.height());

        // Custom badge style
        Core::ItemPainter::BadgeStyle badgeStyle;
        badgeStyle.background = Qt::transparent;
        badgeStyle.text = colors.textSecondary;
        badgeStyle.border = colors.borderSubtle;

        Core::ItemPainter::paintBadge(painter, badgeRect, idText, QIcon(), &badgeStyle);

        // --- Text rect ---
        QRect textRect(badgeRect.right() + spacingBetween, centerY - textHeight / 2, textWidth,
                       textHeight);

        Core::ItemPainter::paintText(painter, textRect, msgName, false, QColor(),
                                     Qt::AlignLeft | Qt::AlignVCenter);
    }

    else if (index.column() == Constants::Columns::SigMin)
    {
        double min = index.data(Qt::DisplayRole).toDouble();
        double max = index.sibling(index.row(), Constants::Columns::SigMax).data().toDouble();
        QString text = Core::formatRange(min, max);

        Core::ItemPainter::paintText(painter, cellRect, text, false, QColor(), Qt::AlignCenter);
    }
    // Length Column
    else if (index.column() == Constants::Columns::SigLength)
    {
        QString val = index.data(Qt::DisplayRole).toString();
        if (!val.isEmpty()) val += Constants::SignalsPage::LengthUnit;
        Core::ItemPainter::paintText(painter, cellRect, val, false, QColor(), Qt::AlignCenter);
    }
    // Unit Column
    else if (index.column() == Constants::Columns::SigUnit)
    {
        QString val = Constants::SignalsPage::DefaultUnit;
        if (!index.data(Role_Unit).toString().isEmpty()) val = index.data(Role_Unit).toString();
        Core::ItemPainter::paintText(painter, cellRect, val, false, QColor(), Qt::AlignCenter);
    }
    // Standard columns
    else
    {
        QString text = index.data(Qt::DisplayRole).toString();

        Core::ItemPainter::paintText(painter, cellRect, text, false, QColor(), Qt::AlignCenter);
    }
}

auto SignalTableDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const -> QSize
{
    const auto& spacing = THEME.spacing();
    return {option.rect.width(), spacing.HeightSm};
}

}  // namespace DbcFile