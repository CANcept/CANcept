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

    const int padding = THEME_INT(Core::Theme::SpacingMd);
    const int radius = THEME_INT(Core::Theme::RadiusMd);
    const QRect rect = option.rect.adjusted(padding / 2, 2, -padding / 2, -2);

    // Defines the states during hovering / when selected
    if (option.state & QStyle::State_Selected)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(THEME_COLOR(Core::Theme::SurfaceSelected));
        painter->drawRoundedRect(rect, radius, radius);

        painter->setBrush(THEME_COLOR(Core::Theme::ColorPrimary));
        painter->drawRoundedRect(rect.left(), rect.top() + padding,
                                 THEME_INT(Core::Theme::BorderThick) * 2,
                                 rect.height() - (padding * 2), 2, 2);
    } else if (option.state & QStyle::State_MouseOver)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(THEME_COLOR(Core::Theme::SurfaceHover));
        painter->drawRoundedRect(rect, radius, radius);
    }

    int iconSize = rect.height() - (padding * 2);
    const QRect iconRect(rect.left() + padding, rect.top() + padding, iconSize, iconSize);

    if (!icon.isNull())
    {
        // Decide icon mode based on state
        const QIcon::Mode mode =
            (option.state & QStyle::State_Selected) ? QIcon::Selected : QIcon::Normal;
        icon.paint(painter, iconRect, Qt::AlignCenter, mode);
    }

    const QColor textColor = (option.state & QStyle::State_Selected)
                                 ? THEME_COLOR(Core::Theme::ColorPrimary)
                                 : THEME_COLOR(Core::Theme::TextPrimary);

    QFont font = painter->font();
    font.setPixelSize(THEME_INT(Core::Theme::SizeTextMd));
    font.setWeight(static_cast<QFont::Weight>((option.state & QStyle::State_Selected)
                                                  ? THEME_INT(Core::Theme::WeightMedium)
                                                  : THEME_INT(Core::Theme::WeightNormal)));
    painter->setFont(font);
    painter->setPen(textColor);

    // Position text to the right of the icon
    const QRect textRect = rect.adjusted(iconSize + (padding * 2), 0, -padding, 0);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    painter->restore();
}

QSize AppRootDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int baseHeight =
        THEME_INT(Core::Theme::SizeTextMd) + (THEME_INT(Core::Theme::SpacingLg) * 2);
    return QSize(200, baseHeight);
}

}  // namespace AppRoot