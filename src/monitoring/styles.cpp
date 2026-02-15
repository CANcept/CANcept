#include "monitoring/styles.hpp"

#include "core/constants.hpp"
#include "core/macro/theme.hpp"

namespace Monitoring::Style {
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

}  // namespace Common
}  // namespace Monitoring::Style