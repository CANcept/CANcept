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