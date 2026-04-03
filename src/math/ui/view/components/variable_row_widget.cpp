#include "math/ui/view/components/variable_row_widget.hpp"

#include <QRegularExpressionValidator>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/common/styled_combo_box.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "math/ui/model/math_input_model.hpp"

namespace Math {

VariableRowWidget::VariableRowWidget(const Providers& providers, QWidget* parent)
    : QWidget(parent), m_providers(providers)
{
    const auto& spacing = THEME.spacing();

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(spacing.spacingSm);

    // Symbol input: single lowercase letter
    m_symbolInput = new Core::StyledLineEdit(this);
    m_symbolInput->setMaxLength(1);
    m_symbolInput->setPlaceholderText("x");
    m_symbolInput->setPadding(spacing.spacingXs, spacing.spacingSm);
    m_symbolInput->setFixedWidth(spacing.spacingXl * 2);
    m_symbolInput->setValidator(
        new QRegularExpressionValidator(QRegularExpression("[a-z]"), m_symbolInput));
    layout->addWidget(m_symbolInput);

    // Type dropdown
    m_typeDropdown = new Core::StyledComboBox(this);
    for (const auto& provider : m_providers)
    {
        m_typeDropdown->addItem(provider->typeName());
    }
    m_typeDropdown->setFixedWidth(spacing.spacingXl * 4);
    connect(m_typeDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VariableRowWidget::onTypeChanged);
    layout->addWidget(m_typeDropdown);

    // Options container
    m_optionsLayout = new QHBoxLayout();
    m_optionsLayout->setContentsMargins(0, 0, 0, 0);
    m_optionsLayout->setSpacing(spacing.spacingSm);
    layout->addLayout(m_optionsLayout, 1);

    // Delete button
    m_deleteButton = new QPushButton(QString::fromUtf8("\u00D7"), this);
    m_deleteButton->setFixedSize(spacing.spacingXl, spacing.spacingXl);
    connect(m_deleteButton, &QPushButton::clicked, this, &VariableRowWidget::removeRequested);
    layout->addWidget(m_deleteButton);

    // Initialize options for the first type
    if (!m_providers.empty())
    {
        onTypeChanged(0);
    }

    applyStyle();
}

void VariableRowWidget::setFromBinding(const VariableBinding& binding)
{
    m_symbolInput->setText(QString(QChar(binding.symbol)));

    if (binding.typeIndex >= 0 && binding.typeIndex < m_typeDropdown->count())
    {
        m_typeDropdown->setCurrentIndex(binding.typeIndex);
    }
}

auto VariableRowWidget::symbol() const -> char
{
    const QString text = m_symbolInput->text();
    if (text.isEmpty()) return '\0';
    return text[0].toLatin1();
}

auto VariableRowWidget::configKey() const -> std::string
{
    const int idx = m_typeDropdown->currentIndex();
    if (idx < 0 || !m_currentOptions) return {};
    return m_providers[static_cast<std::size_t>(idx)]->configKey(m_currentOptions);
}

auto VariableRowWidget::displayName() const -> std::string
{
    const int idx = m_typeDropdown->currentIndex();
    if (idx < 0 || !m_currentOptions) return {};
    return m_providers[static_cast<std::size_t>(idx)]->displayName(m_currentOptions);
}

auto VariableRowWidget::typeIndex() const -> int
{
    return m_typeDropdown->currentIndex();
}

auto VariableRowWidget::createVariable() const -> std::unique_ptr<IVariable>
{
    const int idx = m_typeDropdown->currentIndex();
    if (idx < 0 || !m_currentOptions) return nullptr;
    return m_providers[static_cast<std::size_t>(idx)]->createVariable(m_currentOptions);
}

auto VariableRowWidget::isValid() const -> bool
{
    return symbol() != '\0' && !configKey().empty();
}

void VariableRowWidget::onTypeChanged(int index)
{
    if (m_currentOptions)
    {
        m_optionsLayout->removeWidget(m_currentOptions);
        delete m_currentOptions;
        m_currentOptions = nullptr;
    }

    if (index < 0 || static_cast<std::size_t>(index) >= m_providers.size()) return;

    m_currentOptions = m_providers[static_cast<std::size_t>(index)]->createOptionsWidget(this);
    if (m_currentOptions)
    {
        m_optionsLayout->addWidget(m_currentOptions);
        applyStyle();
    }
}

void VariableRowWidget::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // Compact padding for symbol input to match row height
    m_symbolInput->setPadding(spacing.spacingXs, spacing.spacingSm);

    // Uniform combo box styling within the row (compact padding, matching radius)
    const QString comboStyle = QString(
                                   "QComboBox {"
                                   "  background-color: %1;"
                                   "  color: %2;"
                                   "  border-radius: %3px;"
                                   "  padding: %4px %5px;"
                                   "  font-size: %6px;"
                                   "}"
                                   "QComboBox:hover {"
                                   "  background-color: %7;"
                                   "}"
                                   "QComboBox::drop-down {"
                                   "  border: none;"
                                   "  width: %8px;"
                                   "  padding-right: %5px;"
                                   "}"
                                   "QComboBox::down-arrow {"
                                   "  image: url(%10);"
                                   "}"
                                   "QComboBox QAbstractItemView {"
                                   "  background-color: %7;"
                                   "  color: %2;"
                                   "  border: none;"
                                   "  border-radius: %9px;"
                                   "  outline: none;"
                                   "  padding: %4px;"
                                   "}")
                                   .arg(colors.surfaceSecondary.name())
                                   .arg(colors.textPrimary.name())
                                   .arg(spacing.radiusSm)
                                   .arg(spacing.spacingXs)
                                   .arg(spacing.spacingSm)
                                   .arg(spacing.fontSizeSm)
                                   .arg(colors.surfacePrimary.name())
                                   .arg(spacing.spacingLg)
                                   .arg(spacing.radiusSm)
                                   .arg(Core::Constants::ARROW_DOWN_ICON);

    m_typeDropdown->setStyleSheet(comboStyle);

    // Apply same compact style to any combo boxes inside the options widget
    if (m_currentOptions)
    {
        const auto combos = m_currentOptions->findChildren<QComboBox*>();
        for (auto* combo : combos)
        {
            combo->setStyleSheet(comboStyle);
        }
    }

    // Delete button: transparent, circle on hover
    m_deleteButton->setStyleSheet(QString("QPushButton {"
                                          "   background-color: transparent;"
                                          "   border: none;"
                                          "   font-size: %1px;"
                                          "   font-weight: %2;"
                                          "   color: %3;"
                                          "}"
                                          "QPushButton:hover {"
                                          "   background-color: %4;"
                                          "   border-radius: %5px;"
                                          "}")
                                      .arg(spacing.fontSizeLg)
                                      .arg(spacing.fontWeightBold)
                                      .arg(colors.textSecondary.name())
                                      .arg(colors.surfaceHover.name())
                                      .arg(spacing.radiusSm));
}

bool VariableRowWidget::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace Math
