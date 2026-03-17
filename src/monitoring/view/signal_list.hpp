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
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

class SignalList final : public QWidget
{
    Q_OBJECT
   public:
    explicit SignalList(QWidget* parent, MonitoringModel* model);

    void setupUi();

    void updateViewData();

   signals:
    void signalMonitoringToggled(bool checked, const QString& messageId, const QString& signalName);

   public slots:
    void onDbcChange();

   protected:
    /**
     * @brief Handles the rerender of the AppRootView.
     * @param event The received event.
     */
    auto event(QEvent* event) -> bool override;

   private:
    void applyStyle();
    void populateDecodedFromModel();

    void clearMessages();

    /**
     * @brief Scroll area providing vertical scrolling for the graph list.
     */
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_cardsLayout;
    QWidget* m_scrollContent;
    MonitoringModel* m_model;
    QList<QLabel*> m_signalValues;
    QList<QWidget*> m_signalLists;
};

}  // namespace Monitoring
