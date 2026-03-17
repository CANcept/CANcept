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

#include "core/widgets/card_widget.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Core {

CardWidget::CardWidget(const QString& title, const QString& subtitle, const QString& iconPath,
                       QWidget* parent)
    : QFrame(parent), m_contentLayout(nullptr), m_iconPath(iconPath)
{
    setupUi(title, subtitle, iconPath);
}
auto CardWidget::getTitle() const -> const QLabel*
{
    return m_titleLabel;
}
void CardWidget::setTitle(const QString& title)
{
    if (m_titleLabel) m_titleLabel->setText(title);
}
void CardWidget::setSubtitle(const QString& subtitle)
{
    if (m_subtitleLabel) m_subtitleLabel->setText(subtitle);
}

void CardWidget::setupUi(const QString& title, const QString& subtitle, const QString& iconPath)
{
    const auto& spacing = THEME.spacing();

    setObjectName("card");

    m_contentLayout = new QVBoxLayout(this);
    m_contentLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                        spacing.spacingLg);
    m_contentLayout->setSpacing(spacing.spacingMd);

    if (!title.isEmpty() || !iconPath.isEmpty())
    {
        auto* headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(spacing.spacingSm);

        // Icon
        if (!iconPath.isEmpty())
        {
            m_iconLabel = new QLabel(this);
            headerLayout->addWidget(m_iconLabel, 0, Qt::AlignVCenter);
        }

        // Title
        if (!title.isEmpty())
        {
            m_titleLabel = new QLabel(title, this);
            QFont titleFont = m_titleLabel->font();
            titleFont.setPointSize(spacing.fontSizeSm);
            titleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightNormal));
            m_titleLabel->setFont(titleFont);
            headerLayout->addWidget(m_titleLabel);
        }

        headerLayout->addStretch();
        m_contentLayout->addLayout(headerLayout);
    }

    // Subtitle
    if (!subtitle.isEmpty())
    {
        m_subtitleLabel = new QLabel(subtitle, this);
        QFont subtitleFont = m_subtitleLabel->font();
        subtitleFont.setPointSize(spacing.fontSizeXs);
        subtitleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightMedium));
        m_subtitleLabel->setFont(subtitleFont);
        m_contentLayout->addWidget(m_subtitleLabel);
    }

    applyStyle();
}

void CardWidget::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    setStyleSheet(QString("QFrame#card {"
                          "  background-color: %1;"
                          "  border: %2px solid %3;"
                          "  border-radius: %4px;"
                          "}"
                          "QLabel {"
                          "  color: %5;"
                          "}")
                      .arg(colors.surfaceMain.name(QColor::HexArgb))
                      .arg(spacing.borderThin)
                      .arg(colors.borderSubtle.name(QColor::HexArgb))
                      .arg(spacing.radiusSm)
                      .arg(colors.textPrimary.name()));

    if (m_iconLabel && !m_iconPath.isEmpty())
    {
        constexpr int iconHeight = 20;
        const qreal dpr = devicePixelRatioF();

        const QIcon icon(m_iconPath);
        const QSize actualSize = icon.actualSize(QSize(iconHeight * 2, iconHeight));
        QPixmap pixmap = icon.pixmap(actualSize, dpr);

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), colors.surfaceForeground);
        painter.end();

        m_iconLabel->setPixmap(pixmap);
    }

    if (m_titleLabel)
    {
        m_titleLabel->setStyleSheet(QString("color: %1;").arg(colors.textPrimary.name()));
    }

    if (m_subtitleLabel)
    {
        m_subtitleLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    }
}

bool CardWidget::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QFrame::event(event);
}

}  // namespace Core
