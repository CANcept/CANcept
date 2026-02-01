#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

class SignalList final : public QWidget
{
    Q_OBJECT
   public:
    explicit SignalList(QWidget* parent = nullptr);

    void setupUi();

    void clearMessages();

    void setModel(MonitoringModel* model);

    void populateFromModel();

   private slots:
    void onModelRowsInserted(const QModelIndex& parent, int first, int last);
    void onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

   private:
    /**
     * @brief Scroll area providing vertical scrolling for the graph list.
     */
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_cardsLayout;
    QWidget* m_scrollContent;
    MonitoringModel* m_model;
};

}  // namespace Monitoring
