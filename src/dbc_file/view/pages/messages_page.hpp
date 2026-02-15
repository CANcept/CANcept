#pragma once

#include <QComboBox>
#include <QListView>
#include <QSplitter>

// Forward declarations
class QTableView;
class QAbstractItemModel;
class QModelIndex;

namespace Core {
class CardWidget;
class SearchableFilterTable;
}  // namespace Core

namespace DbcFile {

// ==============================================================================
// 1. MessageDetailView
// ==============================================================================

/**
 * @class MessageDetailView
 * @brief Displays detailed information about a selected CAN message.
 *
 * This widget represents the lower part of the MessagesPage (detail pane).
 * It consists of:
 * - A header section (title + subtitle inside a CardWidget)
 * - A QListView showing all signal children of the selected message
 *
 * The list view operates by setting a root index in the source model,
 * which allows displaying only the direct signal children of the message.
 */
class MessageDetailView : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the detail view UI.
     * @param parent Optional parent widget.
     */
    explicit MessageDetailView(QWidget* parent = nullptr);

    /**
     * @brief Returns the internal signal list view.
     *
     * Used by the page to assign models and delegates.
     */
    [[nodiscard]] auto getSignalList() const -> QListView*;

    /**
     * @brief Sets the root index of the signal list.
     * @param index Source model index of the selected message.
     *
     * The list view will display only the direct children (signals)
     * of this index.
     */
    void setRootIndex(const QModelIndex& index);

    /**
     * @brief Updates the header metadata for the selected message.
     *
     * @param name   Message name.
     * @param id     Message identifier.
     * @param sender Sender ECU name.
     * @param dlc    Data length code.
     */
    void updateHeaderInfo(const QString& name, uint id, const QString& sender, int dlc);

   private:
    /** @brief Builds the widget layout. */
    void setupUi();

   private:
    Core::CardWidget* m_card = nullptr;
    QListView* m_signalList = nullptr;
};

// ==============================================================================
// 2. MessagesPage
// ==============================================================================

/**
 * @class MessagesPage
 * @brief Main page of the Messages tab.
 *
 * Implements a vertical master-detail layout using QSplitter:
 *
 * Top section:
 *   - Searchable and filterable table of messages.
 *
 * Bottom section:
 *   - MessageDetailView displaying the selected message and its signals.
 *
 * The master table usually operates on a flat proxy model.
 * The detail view operates on the source tree model using root indices.
 */
class MessagesPage : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the Messages page UI.
     * @param parent Optional parent widget.
     */
    explicit MessagesPage(QWidget* parent = nullptr);

    ~MessagesPage() override = default;

    /**
     * @brief Selects a message for detail display.
     * @param index Source model index of the message.
     */
    void selectMessageIndex(const QModelIndex& index);

    /**
     * @brief Sets the model for the master table (top section).
     *
     * Typically a FlatListProxy showing messages.
     */
    void setMasterModel(QAbstractItemModel* model);

    /**
     * @brief Configures column visibility and resize behavior of the master table.
     */
    void configureMasterColumns(QTableView* table, const QAbstractItemModel* model);

    /**
     * @brief Sets the model for the detail view (bottom section).
     *
     * Usually the full tree model so the list can use root indices.
     */
    void setDetailModel(QAbstractItemModel* model);

    /**
     * @brief Populates the sender filter combo box.
     */
    auto setAvailableSenders(const QStringList& senders) -> void;

   signals:
    /**
     * @brief Emitted when the user selects a message in the master table.
     */
    void messageSelectionChanged(const QModelIndex& proxyIndex);

    /**
     * @brief Forwarded search text change from the filter table.
     */
    void masterFilterTextChanged(const QString& text);

    /**
     * @brief Emitted when the sender filter changes.
     */
    void filterSenderChanged(const QString& sender);

   private slots:
    /**
     * @brief Handles selection changes in the master table.
     */
    void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

    /**
     * @brief Handles changes in the sender filter combo box.
     */
    void onFilterIndexChanged(int index);

   private:
    /** @brief Builds the page layout. */
    void setupUi();

    /** @brief Applies base configuration to the master table. */
    void configureMasterTable();

   private:
    QSplitter* m_splitter = nullptr;
    Core::SearchableFilterTable* m_messagesTable = nullptr;
    MessageDetailView* m_detailView = nullptr;
};

}  // namespace DbcFile
