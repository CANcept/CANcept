#include "dbc_file/styles.hpp"

#include "core/constants.hpp"
#include "core/macro/theme.hpp"

namespace DbcFile::Style {

// ------------------- Common -------------------
namespace Common {

QString verticalScrollBar()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    return QString(R"(
        QScrollBar:vertical {
            background: %1;
            width: %2px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: %3;
            min-height: %4px;
            border-radius: %5px;
        }
        QScrollBar::handle:vertical:hover {
            background: %6;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )")
        .arg(colors.surfaceMain.name(QColor::HexArgb))       // background
        .arg(spacing.WidthXs / 10)                           // width
        .arg(colors.surfaceSecondary.name(QColor::HexArgb))  // handle
        .arg(spacing.HeightSm)                               // min height
        .arg(spacing.radiusSm / 2)                           // handle radius
        .arg(colors.surfaceHover.name());                    // hover color
}

QString emptyLabel()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    return QString("color: %1; font-size: %2px;")
        .arg(colors.textSecondary.name())
        .arg(spacing.fontSizeMd);
}

}  // namespace Common

// ------------------- LoadPage -------------------
namespace LoadPage {

QString uploadZone()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    return QString(
               "#UploadZone { "
               "  background-color: %1;"
               "  border: %2px solid %3;"
               "  border-radius: %4px;"
               "}"
               "#UploadZone:hover { "
               "  background-color: %5;"
               "  border-color: %3;"
               "}"
               "#UploadZone[dragState=\"valid\"] { "
               "  border: %2px solid %6;"
               "}"
               "#UploadZone[dragState=\"invalid\"] { "
               "  border: %2px solid %7;"
               "}")
        .arg(colors.surfaceMain.name(QColor::HexArgb))
        .arg(spacing.borderThick)
        .arg(colors.borderStrong.name(QColor::HexArgb))
        .arg(spacing.radiusSm)
        .arg(colors.surfaceHover.name(QColor::HexArgb))
        .arg(colors.statusSuccess.name(QColor::HexArgb))
        .arg(colors.statusError.name(QColor::HexArgb));
}

QString uploadInstruction()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    return QString(
               "font-size: %1px;"
               "font-weight: %2;"
               "color: %3;")
        .arg(spacing.fontSizeSm)
        .arg(spacing.fontWeightNormal)
        .arg(colors.textSecondary.name());
}

QString statusLabel(bool isError)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    const QString color = isError ? colors.statusError.name() : colors.statusSuccess.name();

    return QString(
               "font-size: %1px;"
               "font-weight: %2;"
               "margin-top: %3px;"
               "color: %4;")
        .arg(spacing.fontSizeSm)
        .arg(spacing.fontWeightNormal)
        .arg(spacing.spacingSm)
        .arg(color);
}

}  // namespace LoadPage

// ------------------- OverviewPage -------------------
namespace OverviewPage {

QString scrollArea()
{
    const auto& colors = THEME.colors();
    return QString("background-color: %1; width: 0px;").arg(colors.surfaceMain.name());
}

QString secondaryLabel()
{
    const auto& colors = THEME.colors();
    return QString("color: %1;").arg(colors.textSecondary.name());
}

QString statTitle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    return QString("color: %1; font-size: %2px;")
        .arg(colors.textPrimary.name())
        .arg(spacing.fontSizeMd);
}

QString statValue()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    return QString("color: %1; font-weight: %2; font-size: %3px;")
        .arg(colors.textPrimary.name())
        .arg(spacing.fontWeightNormal)
        .arg(spacing.fontSizeLg);
}

}  // namespace OverviewPage

// ------------------- EcusPage -------------------
namespace EcusPage {

QString treeStyle()
{
    return QString(R"(
        QTreeView {
            background: transparent;
            border: none;
            outline: none;
        }
        QTreeView::item {
            border: none;
        }
        QTreeView::branch {
            width: 18px;
            height: 18px;
        }
        QTreeView::branch:closed:has-children {
            image: url(%1);
        }
        QTreeView::branch:open:has-children {
            image: url(%2);
        }
    )")
        .arg(Core::Constants::ARROW_RIGHT_ICON, Core::Constants::ARROW_DOWN_ICON);
}

}  // namespace EcusPage

// ------------------- MessagesPage -------------------
namespace MessagesPage {

QString detailList()
{
    return QStringLiteral("background: transparent; border: none;");
}

}  // namespace MessagesPage

}  // namespace DbcFile::Style