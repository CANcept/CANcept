#include "core/widgets/styled_combo_box.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"

namespace Core {

StyledComboBoxDelegate::StyledComboBoxDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void StyledComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_HasFocus;

    // Draw background for hover/selected states with rounded corners
    if (opt.state & (QStyle::State_Selected | QStyle::State_MouseOver))
    {
        painter->setBrush(colors.surfaceSecondary);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(opt.rect, spacing.radiusSm, spacing.radiusSm);
    }
    painter->setPen(colors.textPrimary);
    const QRect textRect = opt.rect.adjusted(spacing.spacingMd, spacing.spacingSm,
                                             -spacing.spacingMd, -spacing.spacingSm);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                      index.data(Qt::DisplayRole).toString());

    painter->restore();
}

QSize StyledComboBoxDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(spacing.spacingLg + spacing.spacingSm * 2);
    return size;
}

StyledComboBox::StyledComboBox(QWidget* parent) : QComboBox(parent)
{
    applyStyle();

    // Configure the popup view with custom delegate
    if (view())
    {
        view()->setItemDelegate(new StyledComboBoxDelegate(this));
        view()->setStyleSheet("QAbstractItemView { border: none; outline: none; }");
        view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
        view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view()->setSelectionBehavior(QAbstractItemView::SelectRows);
        view()->setFocusPolicy(Qt::NoFocus);
    }
}

void StyledComboBox::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString style = QString(
                              "QComboBox {"
                              "  background-color: %1;"
                              "  color: %2;"
                              "  border-radius: %3px;"
                              "  padding: %4px %5px;"
                              "  font-size: %6px;"
                              "}"
                              "QComboBox:hover {"
                              "  background-color: %7;"
                              "}"
                              "QComboBox::drop-down {"
                              "  border: none;"
                              "  width: %8px;"
                              "  padding-right: %5px;"
                              "}"
                              "QComboBox::down-arrow {"
                              "  image: url(%9);"
                              "}"
                              "QComboBox QAbstractItemView {"
                              "  background-color: %7;"
                              "  color: %2;"
                              "  border: none;"
                              "  border-radius: %10px;"
                              "  outline: none;"
                              "  padding: %4px;"
                              "}")
                              .arg(colors.surfaceSecondary.name())
                              .arg(colors.textPrimary.name())
                              .arg(spacing.radiusMd)
                              .arg(spacing.spacingSm)
                              .arg(spacing.spacingMd)
                              .arg(spacing.fontSizeSm)
                              .arg(colors.surfacePrimary.name())
                              .arg(spacing.spacingXl)
                              .arg(Constants::ARROW_DOWN_ICON)
                              .arg(spacing.radiusSm);
    setStyleSheet(style);
}

void StyledComboBox::showPopup()
{
    QComboBox::showPopup();

    if (QWidget* popup = findChild<QFrame*>())
    {
        // Position popup below the combo box
        const QPoint pos = mapToGlobal(QPoint(0, height()));
        popup->move(pos);
        popup->setStyleSheet("QFrame { border: none; }");
    }
}

}  // namespace Core
