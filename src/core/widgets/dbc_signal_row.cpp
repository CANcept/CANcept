#include "dbc_signal_row.hpp"

#include <QDoubleValidator>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "card_widget.hpp"
#include "common/styled_checkbox.hpp"
#include "common/styled_line_edit.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Core {

DbcSignalRowWidget::DbcSignalRowWidget(const QString& name, const QString& unit, double min,
                                       double max, const Config& config, QWidget* parent)
    : QWidget(parent),
      m_cardContainer(nullptr),
      m_selectionCheckbox(nullptr),
      m_nameLabel(nullptr),
      m_rangeLabel(nullptr),
      m_valueEditor(nullptr),
      m_unitLabel(nullptr),
      m_funcToggle(nullptr),
      m_minValue(min),
      m_maxValue(max)
{
    setupUi(name, unit, min, max, config);
}

DbcSignalRowWidget::DbcSignalRowWidget(const QString& name, const QString& unit, const double min,
                                       const double max, QWidget* parent)
    : QWidget(parent),
      m_cardContainer(nullptr),
      m_selectionCheckbox(nullptr),
      m_nameLabel(nullptr),
      m_rangeLabel(nullptr),
      m_valueEditor(nullptr),
      m_unitLabel(nullptr),
      m_funcToggle(nullptr),
      m_minValue(min),
      m_maxValue(max)
{
    Config defaultConfig;
    defaultConfig.mode = Mode::Full;
    defaultConfig.showRange = true;
    defaultConfig.showSelectionCheckbox = false;
    setupUi(name, unit, min, max, defaultConfig);
}

void DbcSignalRowWidget::setupUi(const QString& name, const QString& unit, double min, double max,
                                 const Config& config)
{
    if (config.mode == Mode::Full)
    {
        setupFullMode(name, unit, min, max, config);
    } else
    {
        setupSelectionMode(name, unit, config);
    }
}

void DbcSignalRowWidget::setupFullMode(const QString& name, const QString& unit, const double min,
                                       const double max, const Config& config)
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_cardContainer = new CardWidget(QString(), QString(), QString(), this);
    auto* cardLayout = m_cardContainer->contentLayout();

    if (!cardLayout)
    {
        return;
    }

    cardLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    cardLayout->setSpacing(spacing.spacingSm);

    auto* firstRow = new QHBoxLayout();
    firstRow->setSpacing(spacing.spacingMd);

    m_selectionCheckbox = new StyledCheckBox(m_cardContainer);
    m_selectionCheckbox->setChecked(false);
    firstRow->addWidget(m_selectionCheckbox);

    m_nameLabel = new QLabel(name, m_cardContainer);
    firstRow->addWidget(m_nameLabel);
    firstRow->addStretch();

    if (config.showRange)
    {
        const QString rangeText =
            QString("%1-%2 %3").arg(min, 0, 'f', 0).arg(max, 0, 'f', 0).arg(unit);
        m_rangeLabel = new QLabel(rangeText, m_cardContainer);
        firstRow->addWidget(m_rangeLabel);
    }
    cardLayout->addLayout(firstRow);
    cardLayout->addSpacing(spacing.spacingXs);

    auto* secondRow = new QHBoxLayout();
    secondRow->setSpacing(spacing.spacingSm);
    secondRow->addSpacing(spacing.spacingMd + spacing.spacingMd);

    // Value edit for the corresponding signal
    m_valueEditor = new StyledLineEdit(m_cardContainer);
    m_valueEditor->setFixedHeight(spacing.spacingXl + spacing.spacingMd);
    m_valueEditor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    double exampleValue = 0.0;
    if (min >= 0)
    {
        exampleValue = (min + max) / 2.0;
    } else if (max > 0)
    {
        exampleValue = 0.0;
    } else
    {
        exampleValue = (min + max) / 2.0;
    }
    m_valueEditor->setPlaceholderText(QString("e.g., %1").arg(exampleValue, 0, 'f', 0));
    m_valueEditor->setText(QString());
    m_valueEditor->setStyleSheet(
        m_valueEditor->styleSheet() +
        QString("QLineEdit { padding: %1px %2px; }").arg(spacing.spacingXs).arg(spacing.spacingMd));
    auto* validator = new QDoubleValidator(min, max, 6, m_valueEditor);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setLocale(QLocale::C);
    m_valueEditor->setValidator(validator);

    // Connect real-time clamping - prevent values outside range
    connect(m_valueEditor, &QLineEdit::textChanged, this, &DbcSignalRowWidget::clampInput);

    secondRow->addWidget(m_valueEditor, 1);

    m_unitLabel = new QLabel(unit, m_cardContainer);
    m_unitLabel->setMinimumWidth(40);
    secondRow->addWidget(m_unitLabel);

    cardLayout->addLayout(secondRow);

    mainLayout->addWidget(m_cardContainer);

    applyStyle();
}

void DbcSignalRowWidget::setupSelectionMode(const QString& name, const QString& unit,
                                            const Config& config)
{
    const auto& spacing = THEME.spacing();

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, spacing.spacingXs / 2, 0, spacing.spacingXs / 2);
    layout->setSpacing(spacing.spacingSm);

    // Optional styled selection checkbox
    if (config.showSelectionCheckbox)
    {
        m_selectionCheckbox = new StyledCheckBox(this);
        layout->addWidget(m_selectionCheckbox);
    }

    // Signal name
    m_nameLabel = new QLabel(name, this);
    layout->addWidget(m_nameLabel);

    layout->addStretch();

    // Unit label (if provided)
    if (!unit.isEmpty())
    {
        m_unitLabel = new QLabel(unit, this);
        layout->addWidget(m_unitLabel);
    }

    applyStyle();
}

void DbcSignalRowWidget::applyStyle() const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    if (m_nameLabel)
    {
        m_nameLabel->setStyleSheet(
            QString("QLabel { font-size: %1px; font-weight: %2; color: %3; }")
                .arg(spacing.fontSizeSm)
                .arg(spacing.fontWeightNormal)
                .arg(colors.textPrimary.name()));
    }

    if (m_rangeLabel)
    {
        m_rangeLabel->setStyleSheet(QString("QLabel { color: %1; font-size: %2px; }")
                                        .arg(colors.textDisabled.name())
                                        .arg(spacing.fontSizeXs));
    }

    if (m_unitLabel)
    {
        m_unitLabel->setStyleSheet(QString("QLabel { color: %1; font-size: %2px; }")
                                       .arg(colors.textSecondary.name())
                                       .arg(spacing.fontSizeSm));
    }

    if (m_valueEditor)
    {
        m_valueEditor->setFixedHeight(spacing.spacingXl + spacing.spacingMd);
        m_valueEditor->setStyleSheet(m_valueEditor->styleSheet() +
                                     QString("QLineEdit { padding: %1px %2px; }")
                                         .arg(spacing.spacingXs)
                                         .arg(spacing.spacingMd));
    }
}

bool DbcSignalRowWidget::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void DbcSignalRowWidget::clampInput() const
{
    if (!m_valueEditor)
    {
        return;
    }

    const QString text = m_valueEditor->text().trimmed();
    if (text.isEmpty())
    {
        return;
    }

    bool ok = false;
    const double value = text.toDouble(&ok);

    if (!ok)
    {
        return;
    }

    // If value exceeds max, clamp it
    if (value > m_maxValue)
    {
        m_valueEditor->blockSignals(true);
        m_valueEditor->setText(QString::number(m_maxValue, 'f', 2));
        m_valueEditor->blockSignals(false);
    } else if (value < m_minValue)
    {
        m_valueEditor->blockSignals(true);
        m_valueEditor->setText(QString::number(m_minValue, 'f', 2));
        m_valueEditor->blockSignals(false);
    }
}

}  // namespace Core
