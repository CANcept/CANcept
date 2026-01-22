#include "dbc_signal_row.hpp"

#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"

namespace Core {

DbcSignalRowWidget::DbcSignalRowWidget(const QString& name, const QString& unit, double min,
                                       double max, QWidget* parent)
    : QWidget(parent),
      m_nameLabel(nullptr),
      m_rangeLabel(nullptr),
      m_valueEditor(nullptr),
      m_unitLabel(nullptr),
      m_funcToggle(nullptr)
{
    setupUi(name, unit, min, max);
}

void DbcSignalRowWidget::setupUi(const QString& name, const QString& unit, double min, double max)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, spacing.spacingXs, 0, spacing.spacingXs);
    mainLayout->setSpacing(2);

    // === First row: Signal indicator, name, value input, unit ===
    auto* inputRow = new QHBoxLayout();
    inputRow->setSpacing(spacing.spacingSm);

    // Signal indicator (small circle)
    auto* indicator = new QLabel(QString::fromUtf8("\u25CB"), this);  // Empty circle
    indicator->setStyleSheet(QString("color: %1; font-size: %2px;")
                                 .arg(colors.textDisabled.name())
                                 .arg(spacing.fontSizeXs));
    indicator->setFixedWidth(spacing.spacingLg);

    // Signal name
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setStyleSheet(QString("font-size: %1px;").arg(spacing.fontSizeSm));
    m_nameLabel->setMinimumWidth(120);

    // Value input
    m_valueEditor = new QLineEdit(this);
    m_valueEditor->setFixedWidth(80);
    m_valueEditor->setAlignment(Qt::AlignRight);
    m_valueEditor->setText(QString::number(min, 'f', 2));

    // Set validator for numeric input
    auto* validator = new QDoubleValidator(min, max, 4, m_valueEditor);
    validator->setNotation(QDoubleValidator::StandardNotation);
    m_valueEditor->setValidator(validator);

    // Unit label
    m_unitLabel = new QLabel(unit, this);
    m_unitLabel->setStyleSheet(QString("color: %1; font-size: %2px;")
                                   .arg(colors.textSecondary.name())
                                   .arg(spacing.fontSizeXs + 1));
    m_unitLabel->setMinimumWidth(40);

    inputRow->addWidget(indicator);
    inputRow->addWidget(m_nameLabel);
    inputRow->addStretch();
    inputRow->addWidget(m_valueEditor);
    inputRow->addWidget(m_unitLabel);

    mainLayout->addLayout(inputRow);

    // === Second row: Range and function toggle ===
    auto* infoRow = new QHBoxLayout();
    infoRow->setSpacing(spacing.spacingSm);
    infoRow->setContentsMargins(spacing.spacingXl, 0, 0, 0);  // Indent to align with name

    // Range label
    QString rangeText = QString("%1-%2 %3").arg(min, 0, 'f', 1).arg(max, 0, 'f', 1).arg(unit);
    m_rangeLabel = new QLabel(rangeText, this);
    m_rangeLabel->setStyleSheet(QString("color: %1; font-size: %2px;")
                                    .arg(colors.textDisabled.name())
                                    .arg(spacing.fontSizeXs));

    // Function toggle
    m_funcToggle = new QCheckBox(tr("Use value function (Optional)"), this);
    m_funcToggle->setStyleSheet(QString("color: %1; font-size: %2px;")
                                    .arg(colors.textSecondary.name())
                                    .arg(spacing.fontSizeXs));

    infoRow->addWidget(m_rangeLabel);
    infoRow->addStretch();
    infoRow->addWidget(m_funcToggle);

    mainLayout->addLayout(infoRow);
}

}  // namespace Core
