#ifndef CANBUSMANAGER_MONITORING_DELEGATE_HPP
#define CANBUSMANAGER_MONITORING_DELEGATE_HPP

#include <QStyledItemDelegate>

#include "monitoring/view/monitoring_view.hpp"

/**
 * @namespace Monitoring
 * @brief Contains delegate and UI components for CAN signal monitoring.
 */
namespace Monitoring {
/**
 * @class MonitoringDelegate
 * @brief The active controller for the Monitoring module.
 * * Owns the transmission logic, including the cyclic timer. It bridges
 * the passive Model/View to the Component's broker slots.
 */
class MonitoringDelegate : QStyledItemDelegate
{
   public:
    /**
     * @brief Constructs the monitoring delegate.
     *
     * @param parent Optional Qt parent object.
     */
    explicit MonitoringDelegate(QObject* parent = nullptr);

    /**
     * @brief Default destructor.
     */
    ~MonitoringDelegate() override = default;

    /**
     * @brief Returns the formatted display text for a model value.
     *
     * This method is invoked by the view to obtain the user-visible string
     * representation of model data, allowing domain-specific formatting
     * without altering the model itself.
     *
     * @param value  The data value provided by the model.
     * @param locale The locale used for formatting.
     * @return Localized, formatted display string.
     */
    [[nodiscard]] auto displayText(const QVariant& value,
                                   const QLocale& locale) const -> QString override;
    /**
     * @brief Paints a rounded card with an icon and a count badge.
     *
     * @caller Qt View (QListView) during paint events.
     *
     * @details
     * 1. Draws a rounded rectangle background.
     * 2. Draws a placeholder icon on the left.
     * 3. Draws the item name (ECU or Signal) in the center.
     *
     * @param painter The painter object used for drawing.
     * @param option Contains geometry (rect) and state (selected/hovered).
     * @param index The model index providing the data.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Determines the size of the card.
     *
     * @caller Qt View layout system.
     * @return A fixed size (e.g., 200x50px) to ensure proper grid alignment.
     */
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
}  // namespace Monitoring

#endif  // CANBUSMANAGER_MONITORING_DELEGATE_HPP
