#include "math_input_button.hpp"

#include <QPainter>
#include <utility>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Math {

MathInputButton::MathInputButton(QString iconPath, QWidget* parent)
    : QAbstractButton(parent), m_iconPath(std::move(iconPath))
{
    setAttribute(Qt::WA_Hover);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    applyStyle();
}

MathInputButton::MathInputButton(AsLabel, const QString& label, QWidget* parent)
    : QAbstractButton(parent)
{
    setText(label);
    setAttribute(Qt::WA_Hover);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    applyStyle();
}

auto MathInputButton::sizeHint() const -> QSize
{
    const int size = THEME.spacing().spacingLg * 2;
    return {size, size};
}

void MathInputButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_hovered ? m_hover : m_background);
    painter.drawRoundedRect(rect(), THEME.spacing().radiusXs, THEME.spacing().radiusXs);

    if (!m_pixmap.isNull())
    {
        const int padding = THEME.spacing().spacingSm;
        painter.drawPixmap(rect().adjusted(padding, padding, -padding, -padding), m_pixmap);
    } else if (!text().isEmpty())
    {
        painter.setPen(m_textColor);
        painter.setFont(font());
        painter.drawText(rect(), Qt::AlignCenter, text());
    }
}

auto MathInputButton::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        update();
        return true;
    }
    if (event->type() == QEvent::HoverEnter)
    {
        m_hovered = true;
        update();
    }
    if (event->type() == QEvent::HoverLeave)
    {
        m_hovered = false;
        update();
    }
    return QAbstractButton::event(event);
}

void MathInputButton::applyStyle()
{
    const auto& colors = THEME.colors();
    m_background = colors.surfacePrimary;
    m_hover = colors.surfaceHover;
    m_textColor = colors.textPrimary;

    QPixmap pix(m_iconPath);
    if (!pix.isNull())
    {
        QPainter p(&pix);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), colors.textPrimary);
        p.end();
    }
    m_pixmap = pix;
}

}  // namespace Math
