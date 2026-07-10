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

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>

namespace Core {

class DbcSignalRowWidget;  // Forward declaration
class StyledCheckBox;      // Forward declaration

/**
 * @class DbcMessageCard
 * @brief Reusable UI component for displaying a DBC message and its signals.
 *
 * Generic card widget that can be used across different modules:
 * - Sending: Message composition with checkbox for selection
 * - Logging: Message filtering with checkbox for selection
 * - Monitoring: Message display (checkbox can be hidden)
 *
 * The checkbox can be optionally hidden for read-only displays.
 */
class DbcMessageCard : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Configuration for message card appearance and behavior.
     */
    struct Config {
        bool showCheckbox = true;      ///< Show selection checkbox in header
        bool startExpanded = false;    ///< Start with signals visible
        QString checkboxTooltip = "";  ///< Tooltip for checkbox (context-specific)
    };

    /**
     * @brief Constructs a message card with custom configuration.
     * @param name Message name (e.g., "EngineData")
     * @param id CAN message ID (e.g., 0x100)
     * @param signalCount Number of signals in this message
     * @param config Configuration for card behavior
     * @param parent Parent widget
     */
    explicit DbcMessageCard(const QString& name, uint32_t id, int signalCount, const Config& config,
                            QWidget* parent = nullptr);

    /**
     * @brief Constructs a message card with default configuration.
     * @param name Message name (e.g., "EngineData")
     * @param id CAN message ID (e.g., 0x100)
     * @param signalCount Number of signals in this message
     * @param parent Parent widget
     */
    explicit DbcMessageCard(const QString& name, uint32_t id, int signalCount,
                            QWidget* parent = nullptr);

    ~DbcMessageCard() override = default;

    /** @brief Adds a signal row widget to the card body. */
    void addSignalRow(DbcSignalRowWidget* rowWidget);

    /** @brief Clears all signal rows from the card body. */
    void clearSignalRows();

    /** @brief Returns the CAN message ID this card was constructed with. */
    [[nodiscard]] auto messageId() const -> uint32_t
    {
        return m_messageId;
    }

    /** @brief Returns the signal row widgets currently in the card body, e.g. for serialization. */
    [[nodiscard]] auto signalRows() const -> const std::vector<DbcSignalRowWidget*>&
    {
        return m_signalRows;
    }

    [[nodiscard]] StyledCheckBox* headerCheckbox() const
    {
        return m_headerCheckbox;
    }

    [[nodiscard]] bool isExpanded() const
    {
        return m_bodyContainer && m_bodyContainer->isVisible();
    }

    void setHeaderChecked(bool checked) const;
    void setExpanded(bool expanded) const;

    /**
     * @brief Sets all signal checkboxes to the given state.
     */
    void setAllSignalsChecked(bool checked) const;

    /**
     * @brief Updates the header checkbox based on signal selection state.
     * Shows checked if all selected, unchecked if none, partial if some.
     */
    void updateHeaderFromSignals() const;

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi(const QString& name, uint32_t id, int signalCount, const Config& config);
    void applyStyle() const;
    void connectHeaderToSignals();

    QLabel* m_nameLabel;
    QLabel* m_idLabel;
    QLabel* m_signalCountLabel;
    StyledCheckBox* m_headerCheckbox;
    QPushButton* m_expandBtn;

    QWidget* m_bodyContainer;
    QVBoxLayout* m_signalsLayout;
    std::vector<DbcSignalRowWidget*> m_signalRows;
    uint32_t m_messageId = 0;
};

}  // namespace Core