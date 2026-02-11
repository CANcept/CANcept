#pragma once
#include <QSortFilterProxyModel>
#pragma once
namespace DbcFile {
/**
 * @class SingleMessageProxy
 * @brief A proxy that isolates the children of a specific parent node.
 *
 * @details
 * USE CASE:
 * Used for the "Message Detail View" (Bottom Pane of Messages Tab).
 * It displays ALL signals belonging to the currently selected Message.
 *
 * LOGIC:
 * It filters out everything except the direct children (Signals) of the
 * configured parent index.
 */
class SingleMessageProxy : public QSortFilterProxyModel
{
    Q_OBJECT

   public:
    explicit SingleMessageProxy(QObject* parent = nullptr);
    ~SingleMessageProxy() override = default;

    /**
     * @brief Sets the target message whose signals should be displayed.
     * @caller DbcView::onMessageSelected().
     * @param parentIndex The index of the Message in the source model.
     *
     * Calling this triggers an invalidation of the filter.
     */
    void setFilterParentIndex(const QModelIndex& parentIndex);

protected:
    /**
     * @brief Decides if a row is included.
     * @caller Qt Internal (QSortFilterProxyModel).
     *
     * Logic: Returns true ONLY if `sourceParent == m_parentIndex`.
     */
    [[nodiscard]] auto filterAcceptsRow(int sourceRow,
                                        const QModelIndex& sourceParent) const -> bool override;

private:
    QModelIndex m_parentIndex;
};
}