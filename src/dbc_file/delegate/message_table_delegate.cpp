#include "message_table_delegate.hpp"

#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"
#include "core/util/dbc_utils.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace DbcFile {

namespace {

/**
 * @brief Returns the display text for a role, falling back to a default value.
 */
auto textOrDefault(const QModelIndex& index, int role, const QString& fallback) -> QString
{
    const QString value = index.data(role).toString();
    return value.isEmpty() ? fallback : value;
}
}  // namespace

MessageTableDelegate::MessageTableDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void MessageTableDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    // Paint row background and separators.
    const bool selected = option.state & QStyle::State_Selected;
    Core::ItemPainter::paintRow(painter, option.rect, selected, false);

    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const int padding = spacing.spacingSm;
    const QRect cellRect = option.rect.adjusted(padding, 0, -padding, 0);

    switch (index.column())
    {
        case Constants::Columns::MsgName: {
            const QString text = index.data(Qt::DisplayRole).toString();
            Core::ItemPainter::paintText(painter, cellRect, text, false, QColor(),
                                         Qt::AlignHCenter | Qt::AlignVCenter);
            return;
        }

        case Constants::Columns::MsgId: {
            const uint id = index.data(Role_Id).toUInt();
            const QString text = Core::formatId(id);

            const QSize badgeSize = Core::ItemPainter::measureBadge(text);

            // Center the badge within the cell.
            const int x = option.rect.left() + (option.rect.width() - badgeSize.width()) / 2;
            const int y = option.rect.center().y() - (badgeSize.height() / 2);
            const QRect badgeRect(x, y, badgeSize.width(), badgeSize.height());

            Core::ItemPainter::BadgeStyle style;
            style.background = Qt::transparent;
            style.text = colors.textSecondary;
            style.border = colors.borderSubtle;

            Core::ItemPainter::paintBadge(painter, badgeRect, text, QIcon(), &style);
            return;
        }

        case Constants::Columns::MsgDlc: {
            const QString text =
                textOrDefault(index, Role_Dlc, Constants::MessagesPage::DefaultValue);
            Core::ItemPainter::paintText(painter, cellRect, text, false, QColor(),
                                         Qt::AlignHCenter | Qt::AlignVCenter);
            return;
        }

        case Constants::Columns::MsgSender: {
            const QString text =
                textOrDefault(index, Role_Sender, Constants::MessagesPage::DefaultValue);
            Core::ItemPainter::paintText(painter, cellRect, text, false, QColor(),
                                         Qt::AlignHCenter | Qt::AlignVCenter);
            return;
        }

        case Constants::Columns::MsgSigCount: {
            const QString text =
                textOrDefault(index, Role_ChildCount, Constants::MessagesPage::DefaultValue);
            Core::ItemPainter::paintText(painter, cellRect, text, false, QColor(),
                                         Qt::AlignHCenter | Qt::AlignVCenter);
            return;
        }

        default:
            // Unhandled columns are intentionally left empty.
            return;
    }
}

auto MessageTableDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex&) const -> QSize
{
    return {option.rect.width(), THEME.spacing().HeightSm};
}

}  // namespace DbcFile
