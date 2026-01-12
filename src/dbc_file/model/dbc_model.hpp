//
// Created by Adrian Rupp on 25.12.25.
//
#pragma once

#include <QAbstractItemModel>
#include <memory>

// Core Interfaces & Events
#include "core/event/dbc_event.hpp"
#include "core/interface/i_event_broker.hpp"

// Model Internal Components
#include "dbc_item.hpp"
#include "dbc_roles.hpp"

namespace DbcFile {

/**
 * @class DbcModel
 * @brief The hierarchical data model for the DBC file content.
 *
 * This class acts as a "Smart Model":
 * 1. It holds the reference to the Core::IEventBroker.
 * 2. It subscribes to the DbcParsedEvent to automatically update its data
 *    when a file is parsed by the CAN Handler.
 * 3. It serves data to Views and Delegates via standard Qt roles and
 *    custom DbcRoles.
 */
class DbcModel : public QAbstractItemModel
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the model and subscribes to system events.
     * @caller DbcComponent (in constructor).
     * @param broker Reference to the system-wide EventBroker.
     * @param parent Standard Qt parent.
     */
    explicit DbcModel(Core::IEventBroker& broker, QObject* parent = nullptr);
    ~DbcModel() override;

    // --- QAbstractItemModel Interface Implementation ---

    /**
     * @brief Creates an index for the given row/column and parent.
     * @caller Qt Views (QTreeView, QListView) and Proxies during navigation.
     */
    [[nodiscard]] auto index(int row, int column,
                             const QModelIndex& parent) const -> QModelIndex override;

    /**
     * @brief Returns the parent index of the given child.
     * @caller Qt Views (to draw hierarchy lines) and Proxies (to map indices).
     */
    [[nodiscard]] auto parent(const QModelIndex& child) const -> QModelIndex override;

    /**
     * @brief Returns the number of children (rows) for the given parent.
     * @caller Qt Views (to determine scrollbar size) and Proxies (to iterate).
     */
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;

    /**
     * @brief Returns the number of columns (attributes) for the given parent.
     * @caller Qt Views (to draw headers) and Proxies.
     */
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;

    /**
     * @brief Returns the data for a specific index and role.
     *
     * @caller
     * - **Qt Views:** To render text (DisplayRole).
     * - **Delegates:** To format content (DbcRoles::Role_Unit, Role_IsHex).
     * - **Proxies:** To filter data (DbcRoles::Role_ItemType).
     * - **Mapper:** To populate labels (DisplayRole).
     */
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

    /**
     * @brief Returns the header labels for the columns.
     * @caller Qt HeaderViews (horizontal/vertical headers).
     */
    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                  int role = Qt::DisplayRole) const -> QVariant override;

    // --- Helper Methods ---

    /**
     * @brief Helper to retrieve the semantic type (ECU, Message, Signal) of an index.
     * @caller Proxy Models (TreeFilterProxy, FlatListProxy) to decide if a row should be visible.
     * @return The item type enum.
     */
    [[nodiscard]] auto getItemType(const QModelIndex& index) const -> Core::DbcItemType;

   private:
    /**
     * @brief Callback: Triggered when the EventBroker publishes a parsing success event.
     * @caller Core::IEventBroker (via lambda callback).
     * Resets model and calls setupData() to rebuild the tree.
     */
    void onDbcParsed(const Core::DBCParsedEvent& event);

    /**
     * @brief Rebuilds the internal DbcItem tree structure from the DTO.
     * @caller Internal (onDbcParsed).
     */
    void setupData(const Core::DbcConfig& data);

    // --- Members ---

    Core::IEventBroker& m_broker;

    /**
     * @brief RAII handle for the event subscription.
     * Ensures the subscription stays alive as long as the model exists.
     */
    Core::Connection m_dbcParsedConnection;

    std::unique_ptr<DbcItem> m_rootItem;
};

}  // namespace DbcFile