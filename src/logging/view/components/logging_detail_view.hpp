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

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QWidget>

#include "core/widgets/card_widget.hpp"
#include "logging/model/logging_detail_proxy.hpp"
#include "logging/model/logging_model.hpp"

namespace Logging {

/**
 * @brief Detail view for a single log session, showing session metadata and a paged data table.
 *
 * Owns a LoggingDetailProxy and drives it with the prev/next page controls.
 * Emits backRequested() when the user clicks the back button.
 */
class LoggingDetailView final : public QWidget
{
    Q_OBJECT

   public:
    explicit LoggingDetailView(LoggingModel* model, const QModelIndex& sessionIndex,
                               QWidget* parent = nullptr);
    ~LoggingDetailView() override = default;

   signals:
    /** @brief Emitted when the user wants to return to the session history. */
    void backRequested();

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void setupUi(const LogSession* session);
    void updatePaginator() const;
    void applyStyle() const;

    LoggingDetailProxy* m_proxy;

    Core::CardWidget* m_infoCard;
    Core::CardWidget* m_contentCard;
    QGridLayout* m_infoGrid{nullptr};

    QTableView* m_tableView;
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QLabel* m_pageLabel;
    QPushButton* m_backBtn;
};

}  // namespace Logging