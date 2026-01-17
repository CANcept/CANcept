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
     * @return The amount of children the item with the given index has. Zero if index refers to a
     * column other than 0
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

   private:
    /**
     * @brief Callback: Triggered when the EventBroker publishes a parsing success event.
     * @caller Core::IEventBroker (via lambda callback).
     * Resets model and calls setupData() to rebuild the tree.
     */
    void onDbcParsed(const Core::DBCParsedEvent& event);

    /**
     * @brief Initializes the root item of the DBC model.
     *
     * Creates the invisible root item required by the Qt item model infrastructure.
     * The root item does not represent actual data; it only defines the structural
     * layout of the model.
     *
     * The column count of the root item is initialized to the maximum number of
     * columns used by any item type in the model (signals in this case).
     * Qt requires the root item to define a consistent column count so that
     * indices can be created correctly for all child items, regardless of their
     * depth in the tree.
     *
     * All column values of the root item are initialized with empty QVariant
     * instances, as the root item itself is never displayed.
     */
    void setupRoot();

    /**
    * @brief Creates and inserts the overview item at the root level of the model.
    *
    * Builds a single overview item containing global metadata of the parsed DBC
    * file, such as file name, version, number of ECUs, messages, signals, and
    * orphan messages.
    *
    * The overview item is added as a direct child of the root item and provides
    * a high-level summary of the DBC content. It does not have child items itself
    * and serves as an informational entry at the top of the model.
    *
    * @param data Parsed DBC configuration containing metadata, node definitions,
    *             and message/signal descriptions.
    * @param orphanCount The number of orphan messages since this number is not provided in the Dto
    */
    void createOverviewItem(const Core::DbcConfig& data, int orphanCount) const;


    /**
     * @brief Creates ECU items and inserts them as children of the root item.
     *
     * Translates all node definitions from the parsed DBC configuration into
     * ECU items within the model. Each ECU item is added as a direct child of
     * the root item.
     *
     * The returned map allows efficient lookup of ECU items by name and is
     * used to assign messages to their corresponding transmitting ECU.
     *
     * @param data Parsed DBC configuration containing ECU (node) definitions.
     * @return A map associating ECU names with their corresponding model items.
     */
    [[nodiscard]] auto createEcuItems(const Core::DbcConfig& data) const -> QHash<QString, DbcItem*>;

    /**
     * @brief Calculates the total number of signals in the entire file.
     *
     * @details
     * Iterates through all messages in the DTO and sums up their signal list sizes.
     * Used to populate the statistic field in the Overview Item.
     */
    [[nodiscard]] static auto countTotalSignals(const Core::DbcConfig& data) -> size_t;

    /**
     * @brief Transforms Message definitions to DbcItems and attaches them to their sender ECUs.
     *
     * @param data The source DTO.
     * @param ecuMap The lookup table generated by `createEcuItems`.
     * @return The count of "Orphan" messages (messages where `transmitterName` was not found in the map).
     *
     * @details
     * Iterates over `data.messageDefinitions`. For each message:
     * 1. Looks up the sender in `ecuMap`.
     * 2. If found, uses that ECU as parent. If not, uses an OrphanHolder item on ecu level as
     * parent item and increments orphan count.
     * 3. Creates the `DbcItem` (Type `Message`) and calls `createSignalItems` to fill it.
     */
    [[nodiscard]] auto createMessageItems(const Core::DbcConfig& data,
                            const QHash<QString, DbcItem*>& ecuMap) const -> int;

    /**
     * @brief Iteratively creates Signal items and attaches them to the given Message.
     *
     * @param signalDescriptions List of signals from the Message DTO.
     * @param messageItem Pointer to the parent Message Item.
     *
     * @details
     * Static helper to keep `setupData` clean. It loops over the signal list, converts attributes
     * (StartBit, Factor, Enums like ByteOrder) into `QVariant` columns, and appends the
     * new `Signal` items to the `messageItem`.
     */
    static auto createSignalItems(const std::list<Core::DbcSignalDescription>& signalDescriptions,
        DbcItem* messageItem) -> void;
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