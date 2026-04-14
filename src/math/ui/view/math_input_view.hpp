/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QHBoxLayout>
#include <QList>
#include <QWidget>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>

#include "components/math_input_button.hpp"
#include "math/service/value_function.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

class MathInputModel;
class MathExpressionWidget;
class MathInputAdditionalVariables;
class MathInputStatusIndicator;
class VariableRegistry;

class MathInputView final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the complete math input panel with expression area, buttons, and status.
     */
    explicit MathInputView(VariableRegistry& registry, QWidget* parent = nullptr);
    ~MathInputView() override = default;

    /**
     * @brief Updates all variables, re-evaluates the expression, and returns the result.
     *
     * Thread-safe: called from the sending worker thread at up to 1000+ Hz.
     */
    [[nodiscard]] auto lastValue() -> double;

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

    VariableRegistry& m_registry;
    MathInputModel* m_model;
    MathExpressionWidget* m_expressionWidget;
    QHBoxLayout* m_buttonLayout;
    QList<MathInputButton*> m_buttons;
    MathInputButton* m_variablesButton;
    MathInputStatusIndicator* m_statusIndicator;
    QWidget* m_buttonBar = nullptr;

    mutable std::mutex m_evalMutex;
    std::optional<ValueFunction> m_cachedFunction;
    std::atomic<double> m_lastValue{0.0};
};

}  // namespace Math
