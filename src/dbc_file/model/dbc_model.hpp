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

#include <QAbstractItemModel>
#include <memory>

// Core Interfaces & Events
#include "core/event/dbc_event.hpp"

// Model Internal Components
#include "dbc_item.hpp"

namespace DbcFile {

/**
 * @class DbcModel
 * @brief The hierarchical data model for the DBC file content.
 *
 * This class serves as the passive data store (Model in MVC) for the DBC visualization.
 *
 * Responsibilities:
 * - Stores the hierarchical structure of the parsed DBC file (Root -> ECU -> Message -> Signal).
 * - Provides data to Views and Delegates via standard Qt roles and custom DbcRoles.
 * - Handles the internal logic for grouping orphans and calculating statistics.
 *
 * @note This model does not handle event subscriptions. Data must be pushed into it
 *       via `setDbcConfig()`, typically by the controlling component.
 */
class DbcModel : public QAbstractItemModel
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the DBC model.
     * @param parent Optional QObject parent (usually the DbcComponent).
     *
     * Initializes the internal root item structure. The model is initially empty
     * until `setDbcConfig()` is called.
     */
    explicit DbcModel(QObject* parent = nullptr);
    /**
     * @brief Destroys the model.
     *
     * The event subscription connection is released automatically with the model.
     */
    ~DbcModel() override = default;

    /**
     * @brief Populates the model with a new DBC configuration.
     *
     * @details
     * This method performs a full model reset. It clears all existing data,
     * rebuilds the internal item tree from the provided configuration, and
     * notifies all connected views to refresh their content.
     *
     * @param config The parsed DBC configuration data containing ECUs, messages, and signals.
     */
    void setDbcConfig(const Core::DbcConfig& config);

    // --- QAbstractItemModel Interface Implementation ---

    /**
     * @brief Returns the model index for a given row/column under a parent index.
     * @param row Row of the requested child.
     * @param column Column of the requested child.
     * @param parent Parent model index.
     * @return A valid QModelIndex if the child exists; otherwise an invalid index.
     */
    [[nodiscard]] auto index(int row, int column,
                             const QModelIndex& parent) const -> QModelIndex override;

    /**
     * @brief Returns the parent index for a given child index.
     * @param child Child model index.
     * @return The parent QModelIndex, or an invalid index if the child is top-level.
     */
    [[nodiscard]] auto parent(const QModelIndex& child) const -> QModelIndex override;

    /**
     * @brief Returns the number of rows under a given parent.
     * @param parent Parent model index.
     * @return Number of child items.
     *
     * Only column 0 can have children; for parent.column() > 0 this returns 0.
     */
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;

    /**
     * @brief Returns the number of columns for a given parent.
     * @param parent Parent model index.
     * @return Column count appropriate for the item type of @p parent.
     *
     * The column count depends on the parent item type:
     * - ECU -> message columns
     * - Message -> signal columns
     * - Other -> the item's own column count
     *
     * At the root level, a stable column count is returned to keep proxy models
     * and views functional even when different top-level item types are present.
     */
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;

    /**
     * @brief Returns data stored under the given role for the item referred to by @p index.
     *
     * This function provides role-based access to item data in the model.
     * It dispatches primarily on the requested role and applies type-specific
     * handling depending on the underlying DbcItem type.
     *
     * Supported roles include:
     * - Qt::DisplayRole
     * - Qt::DecorationRole
     * - Role_ItemType
     * - Role_ChildCount
     * - Message-specific roles (e.g. Role_Dlc, Role_Sender, Role_Id)
     * - Signal-specific roles (e.g. Role_StartBit, Role_Unit, Role_Min, ...)
     * - ECU-specific roles (e.g. Role_EcuTotalSignals)
     *
     * If a role is not applicable to the item's type, an invalid QVariant is returned.
     *
     * @param index Model index identifying the item.
     * @param role  Data role requested.
     * @return QVariant containing the requested data, or an invalid QVariant if
     *         the role is not supported or the index is invalid.
     */
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

   private:
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
    [[nodiscard]] auto createEcuItems(const Core::DbcConfig& data) const
        -> QHash<QString, DbcItem*>;

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
     * @return The count of "Orphan" messages (messages where `transmitterName` was not found in the
     * map).
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
     * @param msgDesc
     *
     * @details
     * Static helper to keep `setupData` clean. It loops over the signal list, converts attributes
     * (StartBit, Factor, Enums like ByteOrder) into `QVariant` columns, and appends the
     * new `Signal` items to the `messageItem`.
     */
    static auto createSignalItems(const std::list<Core::DbcSignalDescription>& signalDescriptions,
                                  DbcItem* messageItem,
                                  const Core::DbcMessageDescription& msgDesc) -> void;

    /**
     * @brief Populates the model from a DBC configuration.
     * @param data Parsed DBC configuration.
     *
     * This creates ECU items, message items (including orphans), signal items,
     * and an overview metadata item.
     */
    void setupData(const Core::DbcConfig& data);

    // --- Members ---

    /** @brief Root item of the internal tree. Owns all children. */
    std::unique_ptr<DbcItem> m_rootItem;
};

}  // namespace DbcFile