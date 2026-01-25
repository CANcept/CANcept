#include "core/widgets/card_widget.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>

#include "core/macro/theme.hpp"

namespace Core {

CardWidget::CardWidget(const QString& title, const QString& subtitle, const QString& iconPath,
                       QWidget* parent)
    : QFrame(parent), m_contentLayout(nullptr)
{
    setupUi(title, subtitle, iconPath);
}

void CardWidget::setupUi(const QString& title, const QString& subtitle, const QString& iconPath)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    setObjectName("card");

    // Apply card styling/paddding
    setStyleSheet(QString("QFrame#card {"
                          "  background-color: %1;"
                          "  border: %2px solid %3;"
                          "  border-radius: %4px;"
                          "}")
                      .arg(colors.surfaceMain.name(QColor::HexArgb))
                      .arg(spacing.borderThin)
                      .arg(colors.borderSubtle.name(QColor::HexArgb))
                      .arg(spacing.radiusSm));

    m_contentLayout = new QVBoxLayout(this);
    m_contentLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                        spacing.spacingLg);
    m_contentLayout->setSpacing(spacing.spacingMd);

    // Header with optional icon and title
    if (!title.isEmpty() || !iconPath.isEmpty())
    {
        auto* headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(spacing.spacingSm);

        // Icon
        if (!iconPath.isEmpty())
        {
            auto* iconLabel = new QLabel(this);
            constexpr int iconHeight = 20;
            const qreal dpr = devicePixelRatioF();

            const QIcon icon(iconPath);
            const QSize actualSize = icon.actualSize(QSize(iconHeight * 2, iconHeight));
            QPixmap pixmap = icon.pixmap(actualSize, dpr);

            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), colors.surfaceForeground);
            painter.end();

            iconLabel->setPixmap(pixmap);
            headerLayout->addWidget(iconLabel, 0, Qt::AlignVCenter);
        }

        // Title
        if (!title.isEmpty())
        {
            auto* titleLabel = new QLabel(title, this);
            QFont titleFont = titleLabel->font();
            titleFont.setPointSize(spacing.fontSizeMd);
            titleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightNormal));
            titleLabel->setFont(titleFont);
            titleLabel->setStyleSheet(QString("color: %1;").arg(colors.textPrimary.name()));
            headerLayout->addWidget(titleLabel);
        }

        headerLayout->addStretch();
        m_contentLayout->addLayout(headerLayout);
    }

    // Subtitle
    if (!subtitle.isEmpty())
    {
        auto* subtitleLabel = new QLabel(subtitle, this);
        QFont subtitleFont = subtitleLabel->font();
        subtitleFont.setPointSize(spacing.fontSizeXs);
        subtitleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightMedium));
        subtitleLabel->setFont(subtitleFont);
        subtitleLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
        m_contentLayout->addWidget(subtitleLabel);
    }
}

}  // namespace Core
