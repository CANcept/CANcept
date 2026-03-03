#include "dbc_model.hpp"

#include <QIcon>

#include "core/macro/console_logging.hpp"
#include "core/util/dbc_utils.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_roles.hpp"

namespace DbcFile {

namespace {

/**
 * @brief Extracts the DbcItem pointer stored in a model index.
 * @param idx Model index returned by QAbstractItemModel.
 * @return Pointer to the associated DbcItem, or nullptr if the index is invalid.
 *
 * This is a convenience helper that safely unwraps QModelIndex::internalPointer()
 * and performs the required type cast.
 */
auto itemFromIndex(const QModelIndex& idx) -> DbcItem*
{
    return static_cast<DbcItem*>(idx.internalPointer());
}

/**
 * @brief Resolves the parent item for a model operation.
 * @param parent Parent model index.
 * @param root Root item of the model.
 * @return The DbcItem corresponding to @p parent, or @p root if @p parent is invalid.
 *
 * This helper centralizes the common Qt model pattern where an invalid parent
 * index refers to the root item.
 */
auto parentItemFromIndex(const QModelIndex& parent, DbcItem* root) -> DbcItem*
{
    return parent.isValid() ? itemFromIndex(parent) : root;
}
}  // namespace

DbcModel::DbcModel(QObject* parent) : QAbstractItemModel(parent)
{
    m_rootItem = std::make_unique<DbcItem>(QList<QVariant>{"Root"}, Core::DbcItemType::Root);
}

void DbcModel::setDbcConfig(const Core::DbcConfig& config)
{
    beginResetModel();
    setupData(config);
    endResetModel();
}
auto DbcModel::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    // Return an invalid index if the requested index is outside model bounds.
    if (!hasIndex(row, column, parent))
    {
        return {};
    }

    auto* parentItem = parentItemFromIndex(parent, m_rootItem.get());
    return createIndex(row, column, parentItem->child(row));
}

auto DbcModel::parent(const QModelIndex& child) const -> QModelIndex
{
    if (!child.isValid()) return {};

    const auto* childItem = itemFromIndex(child);

    const DbcItem* parentItem = childItem->parent();

    // Root has no parent in the model.
    if (parentItem == m_rootItem.get())
    {
        return {};
    }

    return createIndex(parentItem->row(), 0, const_cast<DbcItem*>(parentItem));
}

auto DbcModel::rowCount(const QModelIndex& parent) const -> int
{
    // Only the first column can have children in a tree model.
    if (parent.column() > 0) return 0;

    const auto* parentItem = parentItemFromIndex(parent, m_rootItem.get());
    if (!parentItem) return 0;

    return parentItem->childCount();
}

auto DbcModel::columnCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid())
    {
        const auto* parentItem = itemFromIndex(parent);

        switch (parentItem->type())
        {
            case Core::DbcItemType::Message:
                return Constants::Columns::SignalColumnCount;

            case Core::DbcItemType::Ecu:
            case Core::DbcItemType::OrphanHolder:

                return Constants::Columns::MsgColumnCount;

            default:
                return parentItem->columnCount();
        }
    }

    // Root level
    return std::max(Constants::Columns::EcuColumnCount, Constants::Columns::OvColumnCount);
}

auto DbcModel::data(const QModelIndex& index, int role) const -> QVariant
{
    // 1. Basic Validity Check
    if (!index.isValid())
    {
        return {};
    }

    const auto* item = static_cast<DbcItem*>(index.internalPointer());
    const auto type = item->type();

    // -------------------------------------------------------------------------
    // DisplayRole (Text Content for the View)
    // -------------------------------------------------------------------------
    if (role == Qt::DisplayRole)
    {
        // Special Case: The Signal Count column for Messages is calculated dynamically.
        if (type == Core::DbcItemType::Message && index.column() == Constants::Columns::MsgSigCount)
        {
            return item->childCount();
        }

        // Standard Case: Retrieve data stored in the item at the specific column.
        return item->data(index.column());
    }

    // -------------------------------------------------------------------------
    // DecorationRole (Icons)
    // -------------------------------------------------------------------------
    if (role == Qt::DecorationRole)
    {
        if (type == Core::DbcItemType::Ecu) return QIcon(Constants::Sidebar::IconEcus);
        if (type == Core::DbcItemType::Message) return QIcon(Constants::Sidebar::IconMessages);
        if (type == Core::DbcItemType::Signal) return QIcon(Constants::Sidebar::IconSignals);
        return {};
    }

    // -------------------------------------------------------------------------
    // Custom Roles
    // -------------------------------------------------------------------------
    switch (role)
    {
        // --- Generic Roles ---
        case Role_ItemType:
            return QVariant::fromValue(type);

        case Role_ChildCount:
            return item->childCount();

        // --- Message Specific Roles ---
        case Role_Dlc:
            return (type == Core::DbcItemType::Message) ? item->data(Constants::Columns::MsgDlc)
                                                        : QVariant{};

        case Role_Sender:
            return (type == Core::DbcItemType::Message) ? item->data(Constants::Columns::MsgSender)
                                                        : QVariant{};

        // --- ECU Specific Roles ---
        case Role_EcuTotalSignals:
            return (type == Core::DbcItemType::Ecu)
                       ? item->data(Constants::Columns::EcuTotalSignals)
                       : QVariant{};

        // --- ID Handling (Message ID or Parent Message ID for signals) ---
        case Role_Id:
            if (type == Core::DbcItemType::Message) return item->data(Constants::Columns::MsgId);
            if (type == Core::DbcItemType::Signal)
            {
                // Signals are always children of Messages in this model structure.
                if (auto* parent = item->parent())
                {
                    return parent->data(Constants::Columns::MsgId);
                }
            }
            return {};

            // --- Signal Specific Roles (Direct Column Access) ---

        case Role_Unit:
            return (type == Core::DbcItemType::Signal) ? item->data(Constants::Columns::SigUnit)
                                                       : QVariant{};
        case Role_StartBit:
            return (type == Core::DbcItemType::Signal) ? item->data(Constants::Columns::SigStartBit)
                                                       : QVariant{};
        case Role_BitLength:
            return (type == Core::DbcItemType::Signal) ? item->data(Constants::Columns::SigLength)
                                                       : QVariant{};
        case Role_Factor:
            return (type == Core::DbcItemType::Signal) ? item->data(Constants::Columns::SigFactor)
                                                       : QVariant{};
        case Role_Offset:
            return (type == Core::DbcItemType::Signal) ? item->data(Constants::Columns::SigOffset)
                                                       : QVariant{};
        case Role_Min:
            return (type == Core::DbcItemType::Signal) ? item->data(Constants::Columns::SigMin)
                                                       : QVariant{};
        case Role_Max:
            return (type == Core::DbcItemType::Signal) ? item->data(Constants::Columns::SigMax)
                                                       : QVariant{};
        case Role_ByteOrder:
            return (type == Core::DbcItemType::Signal)
                       ? item->data(Constants::Columns::SigByteOrder)
                       : QVariant{};
        case Role_ValueType:
            return (type == Core::DbcItemType::Signal)
                       ? item->data(Constants::Columns::SigValueType)
                       : QVariant{};
        case Role_Receivers:
            return (type == Core::DbcItemType::Signal)
                       ? item->data(Constants::Columns::SigReceivers)
                       : QVariant{};

        default:
            return {};
    }
}

void DbcModel::setupRoot()
{
    // Initialize the root row with enough columns to satisfy any view/proxy usage.
    QList<QVariant> rootColumns;
    rootColumns.reserve(Constants::Columns::SignalColumnCount);
    for (int i = 0; i < Constants::Columns::SignalColumnCount; ++i)
    {
        rootColumns << QVariant{};
    }

    m_rootItem = std::make_unique<DbcItem>(rootColumns, Core::DbcItemType::Root);
}

void DbcModel::createOverviewItem(const Core::DbcConfig& data, int orphanCount) const
{
    // Overview row: store metadata in a single item.
    QList<QVariant> metaData;
    metaData.reserve(6);

    metaData.append(QString::fromStdString(data.metaData.fileName));
    metaData.append(QString::fromStdString(data.metaData.version));
    metaData.append(QVariant::fromValue(static_cast<qulonglong>(data.nodeDefinitions.size())));
    metaData.append(QVariant::fromValue(static_cast<qulonglong>(data.messageDefinitions.size())));
    metaData.append(static_cast<qulonglong>(countTotalSignals(data)));
    metaData.append(orphanCount);

    auto overviewItem =
        std::make_unique<DbcItem>(metaData, Core::DbcItemType::Overview, m_rootItem.get());

    // Put overview at the top.
    m_rootItem->prependChild(std::move(overviewItem));
}

auto DbcModel::createEcuItems(const Core::DbcConfig& data) const -> QHash<QString, DbcItem*>
{
    QHash<QString, DbcItem*> ecuMap;

    for (const std::string& ecu : data.nodeDefinitions)
    {
        const QString ecuName = QString::fromStdString(ecu);

        QList<QVariant> ecuData;
        ecuData.reserve(Constants::Columns::EcuColumnCount);
        ecuData.append(ecuName);
        ecuData.append(0);  // total ECU signal count

        auto ecuItem = std::make_unique<DbcItem>(ecuData, Core::DbcItemType::Ecu, m_rootItem.get());
        auto* ecuPtr = ecuItem.get();

        ecuMap.insert(ecuName, ecuPtr);
        m_rootItem->appendChild(std::move(ecuItem));
    }

    return ecuMap;
}

auto DbcModel::countTotalSignals(const Core::DbcConfig& data) -> size_t
{
    size_t count = 0;
    for (const auto& msg : data.messageDefinitions)
    {
        count += msg.signalDescriptions.size();
    }
    return count;
}

auto DbcModel::createMessageItems(const Core::DbcConfig& data,
                                  const QHash<QString, DbcItem*>& ecuMap) const -> int
{
    int orphanCount = 0;

    // Container node for messages whose sender ECU is missing/unknown.
    QList<QVariant> orphanData;
    // orphanData.append(QStringLiteral("Orphan Messages"));
    auto orphanHolder =
        std::make_unique<DbcItem>(orphanData, Core::DbcItemType::OrphanHolder, m_rootItem.get());

    for (const Core::DbcMessageDescription& msg : data.messageDefinitions)
    {
        // Pre-fill all columns to guarantee stable column access by index.
        QList<QVariant> messageData;
        messageData.reserve(Constants::Columns::MsgColumnCount);
        for (int i = 0; i < Constants::Columns::MsgColumnCount; ++i)
        {
            messageData.append(QVariant{});
        }

        // Write fields into fixed column positions.
        messageData[Constants::Columns::MsgName] = QString::fromStdString(msg.messageName);
        messageData[Constants::Columns::MsgId] = msg.messageId;
        messageData[Constants::Columns::MsgDlc] = msg.messageSize;
        messageData[Constants::Columns::MsgSender] = QString::fromStdString(msg.transmitterName);
        // MsgSigCount is intentionally not stored; it is derived from childCount().

        // Choose parent: known ECU or the orphan holder.
        DbcItem* parent = nullptr;
        const QString transmitterName = QString::fromStdString(msg.transmitterName);

        if (ecuMap.contains(transmitterName))
        {
            parent = ecuMap.value(transmitterName);
        } else
        {
            parent = orphanHolder.get();
            ++orphanCount;
        }

        auto messageItem =
            std::make_unique<DbcItem>(messageData, Core::DbcItemType::Message, parent);

        // Create signal children for this message.
        createSignalItems(msg.signalDescriptions, messageItem.get(), msg);

        // Update ECU aggregate counter.
        if (parent->type() == Core::DbcItemType::Ecu)
        {
            const int current = parent->data(Constants::Columns::EcuTotalSignals).toInt();
            parent->setData(Constants::Columns::EcuTotalSignals,
                            current + static_cast<int>(msg.signalDescriptions.size()));
        }

        parent->appendChild(std::move(messageItem));
    }

    // Only add the orphan holder if it actually contains orphans.
    if (orphanCount > 0)
    {
        m_rootItem->appendChild(std::move(orphanHolder));
    }

    return orphanCount;
}

void DbcModel::createSignalItems(const std::list<Core::DbcSignalDescription>& signalDescriptions,
                                 DbcItem* messageItem, const Core::DbcMessageDescription& msgDesc)
{
    for (const Core::DbcSignalDescription& sig : signalDescriptions)
    {
        QList<QVariant> signalData;
        signalData.reserve(Constants::Columns::SignalColumnCount);
        for (int i = 0; i < Constants::Columns::SignalColumnCount; ++i)
        {
            signalData.append(QVariant{});
        }

        signalData[Constants::Columns::SigName] = QString::fromStdString(sig.signalName);
        signalData[Constants::Columns::SigMessage] = QString::fromStdString(msgDesc.messageName);
        signalData[Constants::Columns::SigStartBit] = sig.startBit;
        signalData[Constants::Columns::SigUnit] = QString::fromStdString(sig.unit);
        signalData[Constants::Columns::SigLength] = sig.signalSize;
        signalData[Constants::Columns::SigMin] = sig.minimum;
        signalData[Constants::Columns::SigMax] = sig.maximum;

        // Keep numeric formatting stable and readable.
        signalData[Constants::Columns::SigFactor] = Core::Util::formatNumber(sig.factor);
        signalData[Constants::Columns::SigOffset] = Core::Util::formatNumber(sig.offset);

        signalData[Constants::Columns::SigByteOrder] =
            sig.byteOrder ? Constants::SignalsPage::BigEndIndicator
                          : Constants::SignalsPage::LittleEndIndicator;

        signalData[Constants::Columns::SigValueType] =
            sig.valueType ? Constants::SignalsPage::SignedIndicator
                          : Constants::SignalsPage::UnsignedIndicator;

        QStringList receiverNames;
        for (const std::string& receiverName : sig.receivers)
        {
            receiverNames.append(QString::fromStdString(receiverName));
        }
        signalData[Constants::Columns::SigReceivers] =
            QVariant(QStringList(receiverNames).join(", "));

        auto signalItem =
            std::make_unique<DbcItem>(signalData, Core::DbcItemType::Signal, messageItem);

        messageItem->appendChild(std::move(signalItem));
    }
}

void DbcModel::setupData(const Core::DbcConfig& data)
{
    setupRoot();

    const QHash<QString, DbcItem*> ecuMap = createEcuItems(data);
    const int orphanCount = createMessageItems(data, ecuMap);

    createOverviewItem(data, orphanCount);
}

}  // namespace DbcFile
