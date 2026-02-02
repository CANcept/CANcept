#pragma once

#include <QAbstractListModel>
#include <QList>

#include "core/interface/i_tab_component.hpp"

namespace AppRoot {

/**
 * @class AppRootModel
 * @brief Concrete implementation of the AppRoot data model.
 * @details Manages a collection of ITabComponent instances and provides
 * them to the View/Delegate
 */
class AppRootModel : public QAbstractListModel
{
    Q_OBJECT

   public:
    /**
     * @enum TabRoles
     * @brief Custom roles for accessing ITabComponent specific data.
     */
    enum TabRoles { IdRole = Qt::UserRole + 1, ComponentRole };

    /**
     * @brief Constructs the model.
     * @param parent The parent QObject.
     */
    explicit AppRootModel(QObject* parent = nullptr);

    /**
     * @brief Virtual destructor.
     */
    ~AppRootModel() override = default;

    /**
     * @brief Returns the number of tabs in the model.
     * @param parent The parent index (unused in list models).
     * @return int The count of tab components.
     */
    [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;

    /**
     * @brief Retrieves data for a specific tab and role.
     * @param index The index of the tab.
     * @param role The requested data role (DisplayRole, DecorationRole, etc.).
     * @return QVariant The data associated with the role.
     */
    [[nodiscard]] auto data(const QModelIndex& index,
                            int role = Qt::DisplayRole) const -> QVariant override;

    /**
     * @brief Appends a tab and connects its 'updated()' signal to the model's 'dataChanged'.
     * @param component Pointer to the ITabComponent implementation.
     */
    void addTab(Core::ITabComponent* component);

    /**
     * @brief Replaces a tab in the data through a new one.
     * @param old_tab the old tab to replace
     * @param new_tab the new tab which takes its place
     */
    void replaceTab(Core::ITabComponent* old_tab, Core::ITabComponent* new_tab);

    /**
     * @brief Removes a tab component from the model by its unique ID.
     * @param id The unique identifier string.
     */
    void removeTab(const QString& id);

    /**
     * @brief Utility getter to retrieve the component at a specific row.
     * @param row The row index.
     * @return Core::ITabComponent* The component pointer, or nullptr if out of bounds.
     */
    [[nodiscard]] auto componentAt(int row) const -> Core::ITabComponent*;

   private:
    /**
     * @brief Internal storage of tab components.
     */
    QList<Core::ITabComponent*> m_tabs;
};

}  // namespace AppRoot