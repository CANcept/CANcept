#pragma once

#include <QObject>
#include <QStyledItemDelegate>

#include "logging/model/logging_model.hpp"
#include "logging/view/logging_view.hpp"
#include "logging/view/message_selection_dialog.hpp"

namespace Logging {

/**
 * @class LoggingDelegate
 * @brief The controller logic that bridges the View, Model, and Modal Dialog.
 * * @details
 * It is responisble to route start/stop action, fullfill navigation and style.
 */
class LoggingDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the Delegate.
     */
    explicit LoggingDelegate(QObject* parent = nullptr);

    /**
     * @brief Renders signal tags and action buttons in their respective columns.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Handles mouse clicks to detect if an action button was pressed.
     * @details If a button area is clicked, it emits the corresponding signal.
     */
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

   signals:
    void exportClicked(const QModelIndex& index);
    void detailClicked(const QModelIndex& index);
};

}  // namespace Logging