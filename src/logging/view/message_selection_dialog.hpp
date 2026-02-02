#pragma once

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QStringList>
#include <QVBoxLayout>
#include <map>

#include "core/dto/dbc_dto.hpp"
#include "core/widgets/dbc_message_card.hpp"
#include "signal_selection_widget.hpp"

namespace Logging {

/**
 * @class MessageSelectionDialog
 * @brief A standalone modal window for configuring hardware and message selection.
 * * @details
 * This dialog provides a blocking configuration interface before a log session starts.
 * It features a device selector at the top and a scrollable area for DBC message cards.
 */
class MessageSelectionDialog final : public QDialog
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the modal configuration window.
     * @param parent The LoggingView (ensures the dialog stays centered over the tab).
     */
    explicit MessageSelectionDialog(QWidget* parent = nullptr);

    /** @brief Virtual destructor. */
    ~MessageSelectionDialog() override = default;

    /**
     * @brief Populates the hardware interface dropdown.
     * @param devices List of available CAN channels (e.g., "can0", "vcan1").
     */
    void setAvailableDevices(const QStringList& devices);

    /**
     * @brief Retrieves the user-selected hardware interface.
     * @return The string identifier of the selected device.
     */
    auto getSelectedDevice() const -> QString;

    /**
     * @brief Injects a DBC message card into the scrollable selection list.
     * @param card Pointer to a DbcMessageCard configured in 'Selection' mode.
     */
    void addMessageCard(Core::DbcMessageCard* card);

    /**
     * @brief Removes and deletes all current message cards.
     */
    void clearCards();

    /**
     * @brief Populates the dialog with messages and signals from a DBC configuration.
     * @param config The parsed DBC configuration containing message and signal definitions.
     */
    void setDbcConfig(const Core::DbcConfig& config);

    /**
     * @brief Returns a map of message IDs to their selected signal names.
     * @return Map where key is message ID and value is list of selected signal names.
     */
    std::map<uint32_t, QStringList> getSelectedSignals() const;

   private:
    /** @brief Initializes the structural layout and styling. */
    void setupUi();

    /** @brief Creates a message card with signal selection for a given message. */
    QWidget* createMessageCardWithSignals(const Core::DbcMessageDescription& message);

    QWidget* m_headerWidget;
    QComboBox* m_deviceSelector;
    QDialogButtonBox* m_buttonBox;

    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_scrollLayout;

    // Storage for DBC configuration and signal widgets
    std::map<uint32_t, SignalSelectionWidget*> m_signalWidgets;
};

}  // namespace Logging