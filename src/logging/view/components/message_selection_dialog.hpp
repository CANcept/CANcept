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

#include <QDialog>
#include <QScrollArea>
#include <QStringList>
#include <map>

#include "core/dto/dbc_dto.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_switch.hpp"
#include "core/widgets/dbc_message_card.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "logging/model/logging_model.hpp"
#include "logging/view/components/start_stop_button.hpp"

namespace Logging {

/**
 * @class MessageSelectionDialog
 * @brief Modal dialog for selecting the log session type and DBC signals before starting a session.
 */
class MessageSelectionDialog final : public QDialog
{
    Q_OBJECT

   public:
    explicit MessageSelectionDialog(QWidget* parent = nullptr);
    ~MessageSelectionDialog() override = default;

   public slots:
    /** @brief Populates the dialog with messages and signals from a DBC configuration. */
    void setDbcConfig(const Core::DbcConfig& config);

   signals:
    /** @brief Emitted when the user confirms the selection; carries session type and selected
     * signals. */
    void startRequested(LogSessionType logSessionType,
                        const std::map<uint32_t, QStringList>& selectedSignals);

   protected:
    auto event(QEvent* event) -> bool override;

   private slots:
    void onLogTypeToggle(bool checked);
    void onStartClicked();
    void updateStartButton();

   private:
    void addMessageCard(Core::DbcMessageCard* card) const;
    void clearCards();
    void setupUi();
    void applyStyle();

    StartStopButton* m_startBtn{nullptr};
    QWidget* m_buttonWidget;
    Core::CardWidget* m_messagesCard;
    QLabel* m_rawPlaceholder;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_scrollLayout;

    std::map<uint32_t, Core::DbcMessageCard*> m_messageCards;
    std::map<uint32_t, std::vector<std::pair<QString, Core::DbcSignalRowWidget*>>> m_signalRows;

    QLabel* m_rawLabel;
    Core::StyledSwitch* m_logTypeSwitch;
    QLabel* m_dbcLabel;
};

}  // namespace Logging