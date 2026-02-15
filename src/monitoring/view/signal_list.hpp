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
