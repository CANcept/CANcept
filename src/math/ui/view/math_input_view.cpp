#include "math/ui/view/math_input_view.hpp"

#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>

#include "components/math_expression_widget.hpp"
#include "components/math_input_button.hpp"
#include "components/math_input_status_indicator.hpp"
#include "components/variable_selection_dialog.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "math/constants.hpp"
#include "math/service/variable_registry.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "math/ui/model/token_registry.hpp"
#include "providers/signal_variable_provider.hpp"
#include "providers/time_variable_provider.hpp"

namespace Math {

MathInputView::MathInputView(VariableRegistry& registry, QWidget* parent)
    : QWidget(parent),
      m_registry(registry),
      m_model(new MathInputModel(registry, this)),
      m_expressionWidget(new MathExpressionWidget(m_model, this)),
      m_buttonLayout(nullptr),
      m_variablesButton(new MathInputButton(MathInputButton::AsLabel{}, "α", this)),
      m_statusIndicator(new MathInputStatusIndicator(this))
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingSm, spacing.spacingXs, spacing.spacingSm,
                                   spacing.spacingXs);

    auto* inputRow = new QHBoxLayout();
    inputRow->setContentsMargins(0, 0, 0, 0);
    inputRow->setSpacing(spacing.spacingXs);
    inputRow->addWidget(m_expressionWidget, 1);

    m_variablesButton->setFixedSize(spacing.spacingXl, spacing.spacingXl);

    auto* actionsPanel = new QHBoxLayout();
    actionsPanel->setContentsMargins(0, 0, 0, 0);
    actionsPanel->setSpacing(spacing.spacingXs);
    actionsPanel->addWidget(m_statusIndicator);
    actionsPanel->addWidget(m_variablesButton);
    inputRow->addLayout(actionsPanel);

    mainLayout->addLayout(inputRow, 1);

    m_buttonBar = new QWidget(this);
    setupButtonBar(m_buttonBar);
    mainLayout->addWidget(m_buttonBar);

    // Open variable configuration dialog on "+" button click
    connect(m_variablesButton, &MathInputButton::clicked, this, [this] {
        // Build providers fresh each time (picks up latest DBC config)
        VariableSelectionDialog::Providers providers;
        providers.push_back(std::make_unique<TimeVariableProvider>());
        providers.push_back(std::make_unique<SignalVariableProvider>(m_registry.dbcConfig()));

        if (VariableSelectionDialog dialog(std::move(providers), m_model->variableBindings(),
                                           window());
            dialog.exec() == QDialog::Accepted)
        {
            auto rows = dialog.resultBindings();

            // Acquire each variable from the registry using the correct row index.
            std::vector<VariableBinding> bindings;
            bindings.reserve(rows.size());
            for (auto& [binding, configKey, rowIndex] : rows)
            {
                binding.variable = m_registry.acquire(configKey, [&dialog, idx = rowIndex]() {
                    return dialog.createVariableForRow(idx);
                });
                bindings.push_back(std::move(binding));
            }

            m_model->setVariableBindings(std::move(bindings));
        }
    });
    connect(m_model, &MathInputModel::changed, this, &MathInputView::reparse);

    // Populate token buttons from registry
    for (const auto& entry : TokenRegistry::tokens())
    {
        auto* btn = new MathInputButton(MathInputButton::AsLabel{}, entry.label,
                                        m_buttonLayout->parentWidget());
        connect(btn, &MathInputButton::clicked, this,
                [this, factory = entry.factory] { m_model->insertToken(factory()); });
        m_buttonLayout->insertWidget(m_buttonLayout->count() - 1, btn);
        m_buttons.append(btn);
    }
}

void MathInputView::setupButtonBar(QWidget* buttonBar)
{
    auto* barLayout = new QHBoxLayout(buttonBar);
    barLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(buttonBar);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* scrollContent = new QWidget(scrollArea);
    m_buttonLayout = new QHBoxLayout(scrollContent);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    barLayout->addWidget(scrollArea, 1);
}

void MathInputView::addNodeButton(const QString& iconPath,
                                  std::function<std::unique_ptr<TokenBase>()> tokenFactory)
{
    auto* button = new MathInputButton(iconPath, m_buttonLayout->parentWidget());
    connect(button, &MathInputButton::clicked, this,
            [this, f = std::move(tokenFactory)] { m_model->insertToken(f()); });
    m_buttonLayout->insertWidget(m_buttonLayout->count() - 1, button);
    m_buttons.append(button);
}

void MathInputView::paintEvent(QPaintEvent* event)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal half = spacing.borderThick / 2.0;
    const qreal borderH = m_buttonBar ? m_buttonBar->y() : height();
    painter.setBrush(colors.surfaceMain);
    painter.setPen(QPen(colors.borderSubtle, spacing.borderThick));
    painter.drawRoundedRect(
        QRectF(half, half, width() - spacing.borderThick, borderH - spacing.borderThick),
        spacing.radiusSm, spacing.radiusSm);
}

void MathInputView::applyStyle()
{
    m_expressionWidget->update();
    update();
}

auto MathInputView::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void MathInputView::reparse()
{
    bool wasParsed = false;
    {
        std::lock_guard lock(m_evalMutex);
        wasParsed = m_cachedFunction.has_value() && m_cachedFunction->isParsed();
        m_cachedFunction.reset();
    }

    if (!m_model->root() || !m_model->isComplete())
    {
        m_statusIndicator->setParseResult({false, ""});
        if (wasParsed) emit validityChanged(false);
        return;
    }

    m_statusIndicator->startParsing();
    ValueFunction vf(m_model->root());
    const auto result = vf.parse();
    m_statusIndicator->setParseResult(result);

    if (result.success)
    {
        {
            std::lock_guard lock(m_evalMutex);
            m_cachedFunction = std::move(vf);
            m_registry.updateAll();
            m_lastValue.store(m_cachedFunction->evaluate(), std::memory_order_relaxed);
        }
        if (!wasParsed)
        {
            emit validityChanged(true);
        }
    } else
    {
        if (wasParsed)
        {
            emit validityChanged(false);
        }
    }
}

auto MathInputView::lastValue() -> double
{
    std::lock_guard lock(m_evalMutex);
    if (m_cachedFunction.has_value() && m_cachedFunction->isParsed())
    {
        m_registry.updateAll();
        m_lastValue.store(m_cachedFunction->evaluate(), std::memory_order_relaxed);
    }
    return m_lastValue.load(std::memory_order_relaxed);
}

auto MathInputView::isValid() const -> bool
{
    return m_cachedFunction.has_value() && m_cachedFunction->isParsed();
}

}  // namespace Math
