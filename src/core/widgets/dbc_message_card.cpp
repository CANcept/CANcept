#include "dbc_message_card.hpp"

#include <QHBoxLayout>

#include "core/macro/theme.hpp"

namespace Core {

DbcMessageCard::DbcMessageCard(const QString& name, uint32_t id, int signalCount, QWidget* parent)
    : QWidget(parent),
      m_nameLabel(nullptr),
      m_idLabel(nullptr),
      m_headerCheckbox(nullptr),
      m_expandBtn(nullptr),
      m_bodyContainer(nullptr),
      m_signalsLayout(nullptr)
{
    setupUi(name, id, signalCount);
}

void DbcMessageCard::setupUi(const QString& name, uint32_t id, int signalCount)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Set card styling
    setStyleSheet(QString("DbcMessageCard { "
                          "  background-color: %1; "
                          "  border: %2px solid %3; "
                          "  border-radius: %4px; "
                          "}")
                      .arg(colors.surfaceMain.name())
                      .arg(spacing.borderThin)
                      .arg(colors.borderSubtle.name())
                      .arg(spacing.spacingXs));

    // === Header ===
    auto* header = new QWidget(this);
    header->setStyleSheet(QString("background-color: %1; border-bottom: %2px solid %3;")
                              .arg(colors.surfacePrimary.name())
                              .arg(spacing.borderThin)
                              .arg(colors.borderSubtle.name()));
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(spacing.spacingMd, spacing.spacingSm, spacing.spacingMd,
                                     spacing.spacingSm);
    headerLayout->setSpacing(spacing.spacingMd);

    // Expand/collapse button
    m_expandBtn = new QPushButton(this);
    m_expandBtn->setFixedSize(spacing.spacingXl, spacing.spacingXl);
    m_expandBtn->setFlat(true);
    m_expandBtn->setText(QString::fromUtf8("\u25BC"));  // Down arrow
    m_expandBtn->setStyleSheet(
        QString("QPushButton { border: none; font-size: %1px; }").arg(spacing.fontSizeXs));

    // Message name
    m_nameLabel = new QLabel(name, header);
    m_nameLabel->setStyleSheet(QString("font-weight: %1; font-size: %2px;")
                                   .arg(spacing.fontWeightBold)
                                   .arg(spacing.fontSizeSm + 1));

    // Signal count label
    auto* signalCountLabel = new QLabel(QString("%1 signals").arg(signalCount), header);
    signalCountLabel->setStyleSheet(QString("color: %1; font-size: %2px;")
                                        .arg(colors.textSecondary.name())
                                        .arg(spacing.fontSizeXs + 1));

    // CAN ID label
    m_idLabel = new QLabel(QString("0x%1").arg(id, 3, 16, QChar('0')).toUpper(), header);
    m_idLabel->setStyleSheet(QString("color: %1; font-size: %2px;")
                                 .arg(colors.textDisabled.name())
                                 .arg(spacing.fontSizeXs + 1));

    // Selection checkbox
    m_headerCheckbox = new QCheckBox(header);
    m_headerCheckbox->setToolTip(tr("Select for transmission"));

    headerLayout->addWidget(m_expandBtn);
    headerLayout->addWidget(m_nameLabel);
    headerLayout->addWidget(signalCountLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_idLabel);
    headerLayout->addWidget(m_headerCheckbox);

    mainLayout->addWidget(header);

    // === Body (signals container) ===
    m_bodyContainer = new QWidget(this);
    m_bodyContainer->setStyleSheet(QString("background-color: %1;").arg(colors.surfaceMain.name()));
    m_signalsLayout = new QVBoxLayout(m_bodyContainer);
    m_signalsLayout->setContentsMargins(spacing.spacingXl + spacing.spacingMd, spacing.spacingSm,
                                        spacing.spacingMd,
                                        spacing.spacingSm);  // Indent signals
    m_signalsLayout->setSpacing(spacing.spacingXs);

    mainLayout->addWidget(m_bodyContainer);

    // Connect expand button
    connect(m_expandBtn, &QPushButton::clicked, this,
            [this]() { setExpanded(!m_bodyContainer->isVisible()); });

    // Start collapsed
    m_bodyContainer->setVisible(false);
}

void DbcMessageCard::addSignalRow(QWidget* rowWidget)
{
    if (rowWidget && m_signalsLayout)
    {
        m_signalsLayout->addWidget(rowWidget);
    }
}

void DbcMessageCard::clearSignalRows()
{
    if (!m_signalsLayout)
    {
        return;
    }

    while (m_signalsLayout->count() > 0)
    {
        QLayoutItem* item = m_signalsLayout->takeAt(0);
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }
}

void DbcMessageCard::setHeaderChecked(bool checked)
{
    if (m_headerCheckbox)
    {
        m_headerCheckbox->setChecked(checked);
    }
}

void DbcMessageCard::setExpanded(bool expanded)
{
    if (m_bodyContainer)
    {
        m_bodyContainer->setVisible(expanded);
    }
    if (m_expandBtn)
    {
        // Update arrow direction
        m_expandBtn->setText(expanded ? QString::fromUtf8("\u25BC") : QString::fromUtf8("\u25B6"));
    }
}

}  // namespace Core
