//
// Created by Adrian Rupp on 28.01.26.
//
#include "section_header.hpp"

#include <QVBoxLayout>

#include "core/theme/theme_manager.hpp"
namespace Core {
SectionHeader::SectionHeader(const QString& title, const QString& subtitle, QWidget* parent)
    : QWidget(parent), m_titleText(title), m_subtitleText(subtitle)
{
    setupUi();
}

void SectionHeader::setupUi()
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    // Small spacing between title and subtitle
    layout->setSpacing(spacing.spacingXs);

    // --- Title ---
    m_lblTitle = new QLabel(m_titleText, this);
    m_lblTitle->setStyleSheet(QString(
        "color: %1; font-size: %2px; font-weight: %3;"
    ).arg(colors.textPrimary.name())
     .arg(spacing.fontSizeLg)
     .arg(spacing.fontWeightBold));

    layout->addWidget(m_lblTitle);

    // --- Subtitle ---
    m_lblSubtitle = new QLabel(m_subtitleText, this);
    m_lblSubtitle->setStyleSheet(QString(
        "color: %1; font-size: %2px; font-weight: %3;"
    ).arg(colors.textSecondary.name())
     .arg(spacing.fontSizeSm)
     .arg(spacing.fontWeightNormal));

    m_lblSubtitle->setWordWrap(true);

    // Hide subtitle if empty
    m_lblSubtitle->setVisible(!m_subtitleText.isEmpty());

    layout->addWidget(m_lblSubtitle);
}

void SectionHeader::setTitle(const QString& title) {
    m_titleText = title;
    if (m_lblTitle) m_lblTitle->setText(title);
}

void SectionHeader::setSubtitle(const QString& subtitle) {
    m_subtitleText = subtitle;
    if (m_lblSubtitle) {
        m_lblSubtitle->setText(subtitle);
        m_lblSubtitle->setVisible(!subtitle.isEmpty());
    }
}
}