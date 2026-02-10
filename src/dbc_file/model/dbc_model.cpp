#include "dbc_model.hpp"

#include <QIcon>

#include "core/macro/console_logging.hpp"
#include "dbc_file/constants.hpp"
namespace DbcFile {

DbcModel::DbcModel(Core::IEventBroker& broker, QObject* parent)
    : QAbstractItemModel(parent), m_broker(broker)
{
    m_rootItem = std::make_unique<DbcItem>(QList<QVariant>{"Root"}, Core::DbcItemType::Root);
    m_dbcParsedConnection = m_broker.subscribe<Core::DBCParsedEvent>(
        [this](const Core::DBCParsedEvent& event) { this->onDbcParsed(event); });
}
DbcModel::~DbcModel() = default;

auto DbcModel::index(const int row, const int column,
                     const QModelIndex& parent) const -> QModelIndex
{
    if (!hasIndex(row, column, parent))  // return empty index if index does not exist
    {
        return QModelIndex{};
    }

    DbcItem* parentItem;
    if (!parent.isValid())  // if parent invalid: parent set to root item
    {
        parentItem = m_rootItem.get();
    } else  // if valid: extract item from parent index
    {
        parentItem = static_cast<DbcItem*>(parent.internalPointer());
    }
    if (DbcItem* childItem = parentItem->child(row))
    {
        return createIndex(row, column, childItem);
    }
    return QModelIndex{};  // otherwise return empty index
}
auto DbcModel::parent(const QModelIndex& child) const -> QModelIndex
{
    if (!child.isValid())
    {
        return QModelIndex{};
    }

    const auto* childItem = static_cast<DbcItem*>(child.internalPointer());
    const DbcItem* parentItem = childItem->parent();
    if (parentItem == m_rootItem.get())
    {
        return QModelIndex{};
    }
    return createIndex(parentItem->row(), 0, parentItem);
}
auto DbcModel::rowCount(const QModelIndex& parent) const -> int
{
    if (parent.column() > 0)  // only first column has children
    {
        return 0;
    }

    const DbcItem* parentItem;
    if (!parent.isValid())  // invalid parent index asks for root item
    {
        parentItem = m_rootItem.get();
    } else
    {
        parentItem = static_cast<DbcItem*>(parent.internalPointer());
    }
    return parentItem->childCount();
}
auto DbcModel::columnCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid())
    {
        const auto* parentItem = static_cast<DbcItem*>(parent.internalPointer());

        if (parentItem->type() == Core::DbcItemType::Message) {
            return Constants::Columns::TotalCount;
        }

        return parentItem->columnCount();
    }
    return Constants::Columns::TotalCount;
}
auto DbcModel::data(const QModelIndex& index, const int role) const -> QVariant
{
    if (!index.isValid()) return {};

    const auto* item = static_cast<DbcItem*>(index.internalPointer());
    const auto type = item->type();

    switch (role)
    {
        case Role_ItemType:
            return QVariant::fromValue(type);

        case Role_ChildCount:
            return item->childCount();

        case Qt::DisplayRole:
            return item->data(index.column());

        case Qt::DecorationRole:
            if (type == Core::DbcItemType::Ecu) return QIcon(Constants::Sidebar::IconEcus);
            if (type == Core::DbcItemType::Message) return QIcon(Constants::Sidebar::IconMessages);
            if (type == Core::DbcItemType::Signal) return QIcon(Constants::Sidebar::IconSignals);
            break;
        default:;
    }

    // --- SIGNALE ---
    if (type == Core::DbcItemType::Signal)
    {
        if (role == Role_Unit) {
            return item->data(Constants::Columns::SigUnit);
        }

        if (role == DbcRoles::Role_Id) {
            if (auto parent = item->parent()) {
                return parent->data(Constants::Columns::MsgId);
            }
            return 0; // Fallback
        }

        switch (role)
        {
            case DbcRoles::Role_StartBit:   return item->data(Constants::Columns::SigStartBit);
            case DbcRoles::Role_BitLength:  return item->data(Constants::Columns::SigLength);
            case DbcRoles::Role_Factor:     return item->data(Constants::Columns::SigFactor);
            case DbcRoles::Role_Offset:     return item->data(Constants::Columns::SigOffset);
            case DbcRoles::Role_Min:        return item->data(Constants::Columns::SigMin);
            case DbcRoles::Role_Max:        return item->data(Constants::Columns::SigMax);
            case DbcRoles::Role_ByteOrder:  return item->data(Constants::Columns::SigByteOrder);
            case DbcRoles::Role_ValueType:  return item->data(Constants::Columns::SigValueType);
            case DbcRoles::Role_Receivers:  return item->data(Constants::Columns::SigReceivers);
            default: break;
        }
    }

    // --- MESSAGES ---
    if (type == Core::DbcItemType::Message)
    {
        if (role == Role_IsHex && index.column() == Constants::Columns::MsgId) {
            return true;
        }

        switch (role)
        {
            case DbcRoles::Role_Id:     return item->data(Constants::Columns::MsgId);
            case DbcRoles::Role_Dlc:    return item->data(Constants::Columns::MsgDlc);
            case DbcRoles::Role_Sender: return item->data(Constants::Columns::MsgSender);
            default: break;
        }
    }

    return {};
}
void DbcModel::onDbcParsed(const Core::DBCParsedEvent& event)
{
    beginResetModel();
    setupData(event.config);
    endResetModel();
}
void DbcModel::setupRoot()
{
    QList<QVariant> rootColumns;
    constexpr int columnCount = Constants::Columns::TotalCount;
    for (int i = 0; i < columnCount; i++) rootColumns << QVariant();
    m_rootItem = std::make_unique<DbcItem>(rootColumns, Core::DbcItemType::Root);
}
void DbcModel::createOverviewItem(const Core::DbcConfig& data, int orphanCount) const
{
    QList<QVariant> metaData;
    // add filename to first column
    metaData.append(QString::fromStdString(data.metaData.fileName));
    // add version to second column
    metaData.append(QString::fromStdString(data.metaData.version));
    // add ECU count to third column
    metaData.append(QVariant::fromValue(static_cast<qulonglong>(data.nodeDefinitions.size())));
    // add message count to 4th column
    metaData.append(QVariant::fromValue(static_cast<qulonglong>(data.messageDefinitions.size())));
    // add signal count to 5th column
    metaData.append(static_cast<qulonglong>(countTotalSignals(data)));
    // add orphan messages count to 6th column
    metaData.append(orphanCount);

    std::unique_ptr<DbcItem> overviewItem =
        std::make_unique<DbcItem>(metaData, Core::DbcItemType::Overview, m_rootItem.get());
    m_rootItem->prependChild(std::move(overviewItem));
}
auto DbcModel::createEcuItems(const Core::DbcConfig& data) const -> QHash<QString, DbcItem*>
{
    QHash<QString, DbcItem*> ecuMap;
    for (const std::string& ecu : data.nodeDefinitions)
    {
        QList<QVariant> ecuData;
        QString ecuName = QString::fromStdString(ecu);
        ecuData.append(ecuName);
        ecuData.append(0);  // total ecu signal count
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
    QList<QVariant> orphanData;
    orphanData.append("Orphan Messages:");
    auto orphanHolder =
        std::make_unique<DbcItem>(orphanData, Core::DbcItemType::OrphanHolder, m_rootItem.get());
    for (const Core::DbcMessageDescription msg : data.messageDefinitions)
    {
        QList<QVariant> messageData;
        messageData.append(QString::fromStdString(msg.messageName));
        messageData.append(msg.messageId);
        messageData.append(msg.messageSize);
        QString transmitterName = QString::fromStdString(msg.transmitterName);
        messageData.append(transmitterName);

        DbcItem* parent;
        if (ecuMap.contains(transmitterName))
        {
            parent = ecuMap.value(transmitterName);
        } else
        {
            parent = orphanHolder.get();
            orphanCount++;
        }
        auto messageItem =
            std::make_unique<DbcItem>(messageData, Core::DbcItemType::Message, parent);
        const int signalCount = static_cast<int>(msg.signalDescriptions.size());
        createSignalItems(msg.signalDescriptions, messageItem.get(), msg);
        if (parent->type() == Core::DbcItemType::Ecu)
        {
            int current = parent->data(Constants::Columns::EcuTotalSignals).toInt();
            parent->setData(Constants::Columns::EcuTotalSignals, current + signalCount);
        }
        parent->appendChild(std::move(messageItem));
    }
    if (orphanCount > 0)
    {
        m_rootItem->appendChild(std::move(orphanHolder));
    }
    return orphanCount;
}
void DbcModel::createSignalItems(const std::list<Core::DbcSignalDescription>& signalDescriptions,
                                 DbcItem* messageItem,
                                 const Core::DbcMessageDescription& msgDesc) // Neu: msgDesc Parameter
{
    for (const Core::DbcSignalDescription& sig : signalDescriptions)
    {
        QList<QVariant> signalData;
        for(int i=0; i<Constants::Columns::TotalCount; ++i) signalData.append(QVariant());

        signalData[Constants::Columns::SigName] = QString::fromStdString(sig.signalName);
        signalData[Constants::Columns::SigMessage] = QString::fromStdString(msgDesc.messageName);
        signalData[Constants::Columns::SigStartBit] = sig.startBit;
        signalData[Constants::Columns::SigUnit] = QString::fromStdString(sig.unit);
        signalData[Constants::Columns::SigLength] = sig.signalSize;
        signalData[Constants::Columns::SigMin] = sig.minimum;
        signalData[Constants::Columns::SigMax] = sig.maximum;
        signalData[Constants::Columns::SigFactor]    = QString::number(sig.factor, 'g', 12);
        signalData[Constants::Columns::SigOffset]    = QString::number(sig.offset, 'g', 12);
        signalData[Constants::Columns::SigByteOrder] = sig.byteOrder ? Constants::SignalsPage::BigEndIndicator : Constants::SignalsPage::LittleEndIndicator;
        signalData[Constants::Columns::SigValueType] = sig.valueType ? Constants::SignalsPage::SignedIndicator : Constants::SignalsPage::UnsignedIndicator;
        QStringList receiverNames;
        for (const std::string& receiverName : sig.receivers) {
            receiverNames.append(QString::fromStdString(receiverName));
        }
        signalData[Constants::Columns::SigReceivers] = receiverNames.join(", ");

        auto signalItem = std::make_unique<DbcItem>(signalData, Core::DbcItemType::Signal, messageItem);

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