#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>

#include "core/dto/dbc_dto.hpp"

namespace Logging {

/**
 * @class SignalSelectionTree
 * @brief Widget for displaying and selecting signals from a DBC message.
 *
 * @details
 * This widget shows a list of signals for a specific CAN message.
 * Users can select/deselect individual signals to be logged.
 * Uses a card-based layout similar to signals_page.cpp but optimized for logging selection.
 */
class SignalSelectionTree : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a signal selection widget for a specific message.
     * @param messageId The CAN message ID this widget represents.
     * @param messageName The human-readable name of the message.
     * @param parent Parent widget.
     */
    explicit SignalSelectionTree(uint32_t messageId, const QString& messageName,
                                 QWidget* parent = nullptr);

    ~SignalSelectionTree() override = default;

    /**
     * @brief Populates the widget with signals from a DBC message.
     * @param signalList List of signal descriptions from the parsed DBC file.
     */
    void setSignals(const std::list<Core::DbcSignalDescription>& signalList);

    /**
     * @brief Returns the list of currently selected signal names.
     * @return QStringList of signal names that are checked.
     */
    QStringList getSelectedSignals() const;

    /**
     * @brief Returns the message ID this widget represents.
     * @return The CAN message ID.
     */
    uint32_t getMessageId() const
    {
        return m_messageId;
    }

    /**
     * @brief Selects or deselects all signals.
     * @param checked If true, all signals are selected; if false, all are deselected.
     */
    void setAllSignalsSelected(bool checked);

   signals:
    /**
     * @brief Emitted when the signal selection changes.
     * @param messageId The message ID for which signals changed.
     * @param selectedCount Number of signals currently selected.
     */
    void selectionChanged(uint32_t messageId, int selectedCount);

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle();

    uint32_t m_messageId;
    QString m_messageName;

    QListView* m_signalList;
    QStandardItemModel* m_signalModel;
    QCheckBox* m_selectAllCheckbox;
};

}  // namespace Logging
