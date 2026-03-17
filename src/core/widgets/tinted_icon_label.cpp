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

#include "tinted_icon_label.hpp"

#include <QPainter>
#include <QPixmap>

namespace Core {

TintedIconLabel::TintedIconLabel(const QString& iconPath, const int size, const QColor& color,
                                 QWidget* parent)
    : QLabel(parent), m_path(iconPath), m_color(color), m_size(size)
{
    setAlignment(Qt::AlignCenter);
    updatePixmap();
}

void TintedIconLabel::setColor(const QColor& color)
{
    if (m_color != color)
    {
        m_color = color;
        updatePixmap();
    }
}
void TintedIconLabel::setIconPath(const QString& path)
{
    m_path = path;
}
void TintedIconLabel::setIconSize(const int size)
{
    m_size = size;
}

void TintedIconLabel::updatePixmap()
{
    if (m_path.isEmpty()) return;

    const QIcon icon(m_path);
    if (icon.isNull()) return;

    QPixmap pix = icon.pixmap(m_size, m_size);

    QPainter p(&pix);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pix.rect(), m_color);
    p.end();

    setPixmap(pix);
}

}  // namespace Core