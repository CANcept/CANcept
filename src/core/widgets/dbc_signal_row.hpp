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

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

#include "card_widget.hpp"
#include "math/ui/view/math_input_view.hpp"

namespace Math {
class VariableRegistry;
}

namespace Core {

class StyledLineEdit;
class StyledCheckBox;

/**
 * @class DbcSignalRowWidget
 * @brief Generic signal row widget for DBC signals.
 *
 * Supports multiple display modes:
 * - Full mode (Sending): Signal name, math expression value editor, unit, range, selection
 *   checkbox. Always requires a VariableRegistry to evaluate expressions against.
 * - PlainValue mode (DBC insert strategy): Signal name, plain validated numeric line edit, unit,
 *   range, selection checkbox. No registry involved - no expression support.
 * - Selection mode (Logging/Monitoring): Signal name and optional selection checkbox
 *
 * The widget automatically adapts its layout based on configuration.
 */
class DbcSignalRowWidget final : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Display mode for the signal row.
     */
    enum class Mode { Full, Selection, PlainValue };

    /**
     * @brief Configuration for signal row appearance and behavior.
     */
    struct Config {
        Mode mode = Mode::Full;
        bool showRange = true;
        bool showSelectionCheckbox = false;
    };

    /**
     * @brief Constructs a full-mode signal row with expression editor (requires registry).
     */
    explicit DbcSignalRowWidget(Math::VariableRegistry& registry, const QString& name,
                                const QString& unit, double min, double max,
                                QWidget* parent = nullptr);

    /**
     * @brief Constructs a signal row widget with custom configuration.
     *
     * Not valid for Mode::Full, which requires a registry and must go through the other
     * constructor.
     *
     * @param name Signal name (e.g., "EngineRPM")
     * @param unit Physical unit (e.g., "rpm")
     * @param min Minimum value
     * @param max Maximum value
     * @param config Configuration for signal row behavior
     * @param parent Parent widget
     */
    explicit DbcSignalRowWidget(const QString& name, const QString& unit, double min, double max,
                                const Config& config, QWidget* parent = nullptr);

    [[nodiscard]] auto valueEditor() const -> Math::MathInputView*
    {
        return m_valueEditor;
    }

    /** @brief Returns the signal name this row was constructed with, e.g. for serialization. */
    [[nodiscard]] auto signalName() const -> QString
    {
        return m_signalName;
    }

    /** @brief The plain numeric value editor, only set in Mode::PlainValue. */
    [[nodiscard]] auto valueLineEdit() const -> StyledLineEdit*
    {
        return m_valueLineEdit;
    }

    [[nodiscard]] auto selectionCheckbox() const -> StyledCheckBox*
    {
        return m_selectionCheckbox;
    }

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi(const QString& name, const QString& unit, double min, double max,
                 const Config& config);
    void setupFullMode(const QString& name, const QString& unit, double min, double max,
                       const Config& config);
    void setupPlainValueMode(const QString& name, const QString& unit, double min, double max,
                             const Config& config);
    void setupSelectionMode(const QString& name, const QString& unit, const Config& config);
    void applyStyle() const;

    Math::VariableRegistry* m_registry;
    QString m_signalName;
    CardWidget* m_cardContainer;
    StyledCheckBox* m_selectionCheckbox;
    QLabel* m_nameLabel;
    QLabel* m_rangeLabel;
    Math::MathInputView* m_valueEditor;
    StyledLineEdit* m_valueLineEdit = nullptr;
    QLabel* m_unitLabel;
};

}  // namespace Core