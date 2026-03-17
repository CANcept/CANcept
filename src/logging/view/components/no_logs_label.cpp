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

#include "no_logs_label.hpp"

#include "core/macro/theme.hpp"

namespace Logging {

NoLogsLabel::NoLogsLabel(QWidget* parent) : QLabel("No past logs", parent)
{
    setAlignment(Qt::AlignCenter);
    applyStyle();
    setVisible(false);
}

void NoLogsLabel::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString labelStyle = QString(
                                   "QLabel {"
                                   "   border: none;"
                                   "   font-size: %3px;"
                                   "   font-weight: %1;"
                                   "   color: %2;"
                                   "}")
                                   .arg(spacing.fontWeightMedium)
                                   .arg(colors.textSecondary.name())
                                   .arg(spacing.fontSizeLg);
    setStyleSheet(labelStyle);
}

}  // namespace Logging
