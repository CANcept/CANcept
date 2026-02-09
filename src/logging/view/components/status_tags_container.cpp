#include "status_tags_container.hpp"

#include <QLabel>

#include "core/macro/theme.hpp"

namespace Logging {

StatusTagsContainer::StatusTagsContainer(QWidget* parent) : QWidget(parent)
{
    const auto& spacing = THEME.spacing();
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(spacing.spacingMd);
    m_layout->addStretch();
    setVisible(false);
}

void StatusTagsContainer::updateStatusTags(const QStringList& messages)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // Clear existing tags
    while (m_layout->count() > 1)  // Keep the stretch
    {
        QLayoutItem* item = m_layout->takeAt(0);
        if (item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Add new tags
    for (const QString& message : messages)
    {
        auto* tagLabel = new QLabel(message, this);
        const QString tagStyle = QString(
                                     "QLabel {"
                                     "   background-color: %1;"
                                     "   border-radius: %2px;"
                                     "   padding: %3px;"
                                     "   font-family: 'Roboto';"
                                     "   font-size: 20px;"
                                     "   font-weight: %4;"
                                     "   color: %5;"
                                     "}")
                                     .arg(colors.surfaceSecondary.name())
                                     .arg(spacing.radiusXs)
                                     .arg(spacing.spacingSm)
                                     .arg(spacing.fontWeightNormal)
                                     .arg(colors.textPrimary.name());
        tagLabel->setStyleSheet(tagStyle);
        m_layout->insertWidget(m_layout->count() - 1, tagLabel);
    }
}

}  // namespace Logging
