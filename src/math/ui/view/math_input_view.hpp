#pragma once

#include <QHBoxLayout>
#include <QList>
#include <QWidget>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>

#include "components/math_input_button.hpp"
#include "math/service/value_function.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

class MathInputModel;
class MathExpressionWidget;
class MathInputAdditionalVariables;
class MathInputStatusIndicator;

class MathInputView final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the complete math input panel with expression area, buttons, and status.
     */
    explicit MathInputView(QWidget* parent = nullptr);
    ~MathInputView() override = default;

    /**
     * @brief Returns the most recently evaluated numeric result of the expression.
     */
    [[nodiscard]] auto lastValue() const -> double;

    /**
     * @brief Returns whether the current expression parses and evaluates successfully.
     */
    [[nodiscard]] auto isValid() const -> bool;

    /**
     * @brief Adds a token-insertion button to the button bar with the given icon and factory.
     */
    void addNodeButton(const QString& iconPath,
                       std::function<std::unique_ptr<TokenBase>()> tokenFactory);

   signals:
    void validityChanged(bool valid);

   protected:
    void paintEvent(QPaintEvent* event) override;
    auto event(QEvent* event) -> bool override;

   private:
    void setupButtonBar(QWidget* buttonBar);
    void applyStyle();
    void reparse();

    MathInputModel* m_model;
    MathExpressionWidget* m_expressionWidget;
    QHBoxLayout* m_buttonLayout;
    QList<MathInputButton*> m_buttons;
    MathInputAdditionalVariables* m_variablesButton;
    MathInputStatusIndicator* m_statusIndicator;
    QWidget* m_buttonBar = nullptr;

    std::optional<ValueFunction> m_cachedFunction;
    std::atomic<double> m_lastValue{0.0};
};

}  // namespace Math
