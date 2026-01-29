#include "dbc_message_card.hpp"

#include <QHBoxLayout>

#include "card_widget.hpp"
#include "common/styled_checkbox.hpp"
#include "core/macro/theme.hpp"

namespace Core {

DbcMessageCard::DbcMessageCard(const QString& name, uint32_t id, int signalCount,
                               const Config& config, QWidget* parent)
    : QWidget(parent),
      m_nameLabel(nullptr),
      m_idLabel(nullptr),
      m_headerCheckbox(nullptr),
      m_expandBtn(nullptr),
      m_bodyContainer(nullptr),
      m_signalsLayout(nullptr)
{
    setupUi(name, id, signalCount, config);
}

DbcMessageCard::DbcMessageCard(const QString& name, uint32_t id, int signalCount, QWidget* parent)
    : QWidget(parent),
      m_nameLabel(nullptr),
      m_idLabel(nullptr),
      m_headerCheckbox(nullptr),
      m_expandBtn(nullptr),
      m_bodyContainer(nullptr),
      m_signalsLayout(nullptr)
{
    Config defaultConfig;
    defaultConfig.showCheckbox = true;
    defaultConfig.startExpanded = false;
    defaultConfig.checkboxTooltip = tr("Select for transmission");
    setupUi(name, id, signalCount, defaultConfig);
}

void DbcMessageCard::setupUi(const QString& name, const uint32_t id, const int signalCount,
                             const Config& config)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Message Card
    auto* card = new CardWidget(QString(), QString(), QString(), this);
    auto* cardLayout = card->contentLayout();

    if (!cardLayout)
    {
        return;
    }

    cardLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    cardLayout->setSpacing(spacing.spacingSm);
    auto* headerRow = new QHBoxLayout();
    headerRow->setSpacing(spacing.spacingMd);

    // Expand/collapse arrow
    m_expandBtn = new QPushButton(card);
    m_expandBtn->setFixedSize(spacing.spacingLg, spacing.spacingLg);
    m_expandBtn->setFlat(true);
    m_expandBtn->setText(config.startExpanded ? QString::fromUtf8("\u25BC")
                                              : QString::fromUtf8("\u25B6"));
    m_expandBtn->setStyleSheet(QString("QPushButton { "
                                       "  border: none; "
                                       "  font-size: %1px; "
                                       "  color: %2; "
                                       "  background: transparent; "
                                       "}")
                                   .arg(spacing.fontSizeSm)
                                   .arg(colors.textSecondary.name()));
    headerRow->addWidget(m_expandBtn);

    // Message Name
    m_nameLabel = new QLabel(name, card);
    m_nameLabel->setStyleSheet(QString("QLabel { "
                                       "  font-weight: %1; "
                                       "  font-size: %2px; "
                                       "  color: %3; "
                                       "  text-decoration: none; "
                                       "}")
                                   .arg(spacing.fontWeightBold)
                                   .arg(spacing.fontSizeSm)
                                   .arg(colors.textPrimary.name()));
    headerRow->addWidget(m_nameLabel);

    // CAN ID with 0x prefix
    m_idLabel = new QLabel(QString("0x%1").arg(id, 3, 16, QChar('0')).toUpper(), card);
    m_idLabel->setStyleSheet(QString("QLabel { "
                                     "  color: %1; "
                                     "  font-size: %2px; "
                                     "  text-decoration: none; "
                                     "}")
                                 .arg(colors.textSecondary.name())
                                 .arg(spacing.fontSizeXs));
    headerRow->addWidget(m_idLabel);

    headerRow->addStretch();

    // Signal count (textSecondary)
    auto* signalCountLabel =
        new QLabel(QString("%1 signal%2").arg(signalCount).arg(signalCount != 1 ? "s" : ""), card);
    signalCountLabel->setStyleSheet(QString("QLabel { "
                                            "  color: %1; "
                                            "  font-size: %2px; "
                                            "}")
                                        .arg(colors.textSecondary.name())
                                        .arg(spacing.fontSizeXs));
    headerRow->addWidget(signalCountLabel);

    // Selection checkbox (optional)
    if (config.showCheckbox)
    {
        m_headerCheckbox = new StyledCheckBox(card);
        if (!config.checkboxTooltip.isEmpty())
        {
            m_headerCheckbox->setToolTip(config.checkboxTooltip);
        }
        headerRow->addWidget(m_headerCheckbox);
    }

    cardLayout->addLayout(headerRow);

    // === Row 2: Signal rows container ===
    m_bodyContainer = new QWidget(card);
    m_signalsLayout = new QVBoxLayout(m_bodyContainer);
    m_signalsLayout->setContentsMargins(0, 0, 0, 0);
    m_signalsLayout->setSpacing(spacing.spacingSm);

    cardLayout->addWidget(m_bodyContainer);

    mainLayout->addWidget(card);

    // Connect expand button
    connect(m_expandBtn, &QPushButton::clicked, this,
            [this]() { setExpanded(!m_bodyContainer->isVisible()); });

    // Set initial expand state
    setExpanded(config.startExpanded);
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
