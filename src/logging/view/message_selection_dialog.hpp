#pragma once

#include <QDialog>
#include <QScrollArea>
#include <QStringList>
#include <QVBoxLayout>
#include <map>

#include "core/dto/dbc_dto.hpp"
#include "core/widgets/dbc_message_card.hpp"

namespace Core {
class CardWidget;
}

namespace Logging {

/**
 * @class MessageSelectionDialog
 * @brief A standalone modal window for message selection.
 *
 * @details
 * This dialog provides a blocking configuration interface before a log session starts.
 * It features a scrollable area for DBC message cards where users can select which
 * messages and signals to log. The CAN interface is configured in global settings.
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

    Core::CardWidget* m_messagesCard;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_scrollLayout;

    // Storage for message cards mapped by message ID
    std::map<uint32_t, Core::DbcMessageCard*> m_messageCards;
};

}  // namespace Logging