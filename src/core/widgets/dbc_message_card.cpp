#include "dbc_message_card.hpp"

#include <QHBoxLayout>

#include "card_widget.hpp"
#include "common/styled_checkbox.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "dbc_signal_row.hpp"

namespace Core {

DbcMessageCard::DbcMessageCard(const QString& name, const uint32_t id, int signalCount,
                               const Config& config, QWidget* parent)
    : QWidget(parent),
      m_nameLabel(nullptr),
      m_idLabel(nullptr),
      m_signalCountLabel(nullptr),
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
      m_signalCountLabel(nullptr),
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
    headerRow->addWidget(m_expandBtn);

    // Message Name
    m_nameLabel = new QLabel(name, card);
    headerRow->addWidget(m_nameLabel);

    // CAN ID with 0x prefix
    m_idLabel = new QLabel(QString("0x%1").arg(id, 3, 16, QChar('0')).toUpper(), card);
    headerRow->addWidget(m_idLabel);

    headerRow->addStretch();

    // Signal count
    m_signalCountLabel =
        new QLabel(QString("%1 signal%2").arg(signalCount).arg(signalCount != 1 ? "s" : ""), card);
    headerRow->addWidget(m_signalCountLabel);

    // Selection checkbox (optional)
    if (config.showCheckbox)
    {
        m_headerCheckbox = new StyledCheckBox(card);
        m_headerCheckbox->setTristate(true);
        if (!config.checkboxTooltip.isEmpty())
        {
            m_headerCheckbox->setToolTip(config.checkboxTooltip);
        }
        headerRow->addWidget(m_headerCheckbox);
        connectHeaderToSignals();
    }

    cardLayout->addLayout(headerRow);

    //  Signal rows container
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

    applyStyle();
}

void DbcMessageCard::applyStyle() const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    m_expandBtn->setStyleSheet(QString("QPushButton { "
                                       "  border: none; "
                                       "  font-size: %1px; "
                                       "  color: %2; "
                                       "  background: transparent; "
                                       "}")
                                   .arg(spacing.fontSizeSm)
                                   .arg(colors.textSecondary.name()));

    m_nameLabel->setStyleSheet(QString("QLabel { "
                                       "  font-weight: %1; "
                                       "  font-size: %2px; "
                                       "  color: %3; "
                                       "  text-decoration: none; "
                                       "}")
                                   .arg(spacing.fontWeightMedium)
                                   .arg(spacing.fontSizeSm)
                                   .arg(colors.textPrimary.name()));

    m_idLabel->setStyleSheet(QString("QLabel { "
                                     "  color: %1; "
                                     "  font-size: %2px; "
                                     "  text-decoration: none; "
                                     "}")
                                 .arg(colors.textSecondary.name())
                                 .arg(spacing.fontSizeXs));

    m_signalCountLabel->setStyleSheet(QString("QLabel { "
                                              "  color: %1; "
                                              "  font-size: %2px; "
                                              "}")
                                          .arg(colors.textSecondary.name())
                                          .arg(spacing.fontSizeXs));
}

bool DbcMessageCard::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void DbcMessageCard::addSignalRow(DbcSignalRowWidget* rowWidget)
{
    if (rowWidget && m_signalsLayout)
    {
        m_signalsLayout->addWidget(rowWidget);
        m_signalRows.push_back(rowWidget);

        // Connect signal checkbox to update header state
        if (const auto* signalCheckbox = rowWidget->selectionCheckbox())
        {
            connect(signalCheckbox, &QCheckBox::toggled, this,
                    &DbcMessageCard::updateHeaderFromSignals);
        }
    }
}

void DbcMessageCard::clearSignalRows()
{
    // disconnect signals
    for (const auto* signalRow : m_signalRows)
    {
        if (const auto* cb = signalRow->selectionCheckbox()) disconnect(cb, nullptr, this, nullptr);
    }

    m_signalRows.clear();

    if (!m_signalsLayout)
    {
        return;
    }

    while (m_signalsLayout->count() > 0)
    {
        const QLayoutItem* item = m_signalsLayout->takeAt(0);
        if (QWidget* widget = item->widget())
        {
            widget->blockSignals(true);
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }
}

void DbcMessageCard::setHeaderChecked(const bool checked) const
{
    if (m_headerCheckbox)
    {
        m_headerCheckbox->setChecked(checked);
    }
}

void DbcMessageCard::setExpanded(const bool expanded) const
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

void DbcMessageCard::setAllSignalsChecked(const bool checked) const
{
    for (const auto* signalRow : m_signalRows)
    {
        if (auto* checkbox = signalRow->selectionCheckbox())
        {
            checkbox->blockSignals(true);
            checkbox->setChecked(checked);
            checkbox->blockSignals(false);
        }
    }
}

void DbcMessageCard::updateHeaderFromSignals() const
{
    if (!m_headerCheckbox || m_signalRows.empty())
    {
        return;
    }

    int checkedCount = 0;
    for (const auto* signalRow : m_signalRows)
    {
        if (const auto* checkbox = signalRow->selectionCheckbox())
        {
            if (checkbox->isChecked())
            {
                ++checkedCount;
            }
        }
    }

    m_headerCheckbox->blockSignals(true);
    if (checkedCount == 0)
    {
        m_headerCheckbox->setCheckState(Qt::Unchecked);
    } else if (checkedCount == static_cast<int>(m_signalRows.size()))
    {
        m_headerCheckbox->setCheckState(Qt::Checked);
    } else
    {
        m_headerCheckbox->setCheckState(Qt::PartiallyChecked);
    }
    m_headerCheckbox->blockSignals(false);
}

void DbcMessageCard::connectHeaderToSignals()
{
    if (!m_headerCheckbox)
    {
        return;
    }

    connect(m_headerCheckbox, &QCheckBox::clicked, this, [this]() {
        // When clicked in partial state, select all; otherwise toggle
        const bool selectAll = (m_headerCheckbox->checkState() != Qt::Unchecked);
        setAllSignalsChecked(selectAll);

        // Update header to reflect actual state (all checked or all unchecked)
        m_headerCheckbox->blockSignals(true);
        m_headerCheckbox->setCheckState(selectAll ? Qt::Checked : Qt::Unchecked);
        m_headerCheckbox->blockSignals(false);
    });
}

}  // namespace Core
