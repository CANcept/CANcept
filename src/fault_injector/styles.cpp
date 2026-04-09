/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fault_injector/styles.hpp"

#include "core/macro/theme.hpp"

namespace FaultInjector::Style {
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
}  // namespace FaultInjector::Style