#include "app_root/delegate/app_root_delegate.hpp"

#include <QApplication>
#include <QIcon>
#include <QMetaType>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QVariant>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"

namespace AppRoot {

AppRootDelegate::AppRootDelegate(QObject* parent) : QAbstractItemDelegate(parent) {}

void AppRootDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    // Extract Data from Model
    const QString text = index.data(Qt::DisplayRole).toString();
    const QVariant iconVariant = index.data(Qt::DecorationRole);
    QIcon icon;
    if (iconVariant.isValid())
    {
        if (iconVariant.userType() == QMetaType::QIcon)
        {
            icon = *static_cast<const QIcon*>(iconVariant.constData());
        }
    }

    const int padding = THEME.spacing().spacingSm;
    const int radius = THEME.spacing().radiusMd;
    const QRect rect = option.rect.adjusted(1, 1, -1, -1);

    // Defines the states during hovering / when selected
    if (option.state & QStyle::State_Selected)
    {
        QPen borderPen(THEME.colors().surfaceForeground);
        borderPen.setWidth(THEME.spacing().borderThick);
        painter->setPen(borderPen);

        painter->setBrush(THEME.colors().surfaceMain);

        const QRectF borderRect = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
        painter->drawRoundedRect(borderRect, radius, radius);
    } else if (option.state & QStyle::State_MouseOver)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(THEME.colors().colorPrimaryHover);
        painter->drawRoundedRect(rect, radius, radius);
    }

    const int iconSize = rect.height() - (padding * 2);
    const QRect iconRect(rect.left() + padding * 2, rect.top() + padding, iconSize, iconSize);

    if (!icon.isNull())
    {
        // Decide icon mode based on state
        const QIcon::Mode mode =
            (option.state & QStyle::State_Selected) ? QIcon::Selected : QIcon::Normal;
        icon.paint(painter, iconRect, Qt::AlignCenter, mode);
    }

    QFont font = painter->font();
    font.setPixelSize(THEME.spacing().fontSizeMd);
    font.setWeight(static_cast<QFont::Weight>((option.state & QStyle::State_Selected)
                                                  ? THEME.spacing().fontWeightMedium
                                                  : THEME.spacing().fontWeightNormal));
    painter->setFont(font);
    painter->setPen(THEME.colors().textPrimary);

    // Position text to the right of the icon
    const QRect textRect = rect.adjusted(iconSize + (padding * 3), 0, -padding, 0);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    painter->restore();
}

auto AppRootDelegate::sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const -> QSize
{
    const int baseHeight = THEME.spacing().fontSizeMd + (THEME.spacing().spacingLg * 2);
    return {200, baseHeight};
}

}  // namespace AppRoot