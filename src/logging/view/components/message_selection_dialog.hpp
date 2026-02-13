#pragma once

#include <QComboBox>
#include <QDialog>
#include <QScrollArea>
#include <QStringList>
#include <map>

#include "core/dto/dbc_dto.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/dbc_message_card.hpp"

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

   protected:
    bool event(QEvent* event) override;

   private:
    /** @brief Initializes the structural layout and styling. */
    void setupUi();
    void applyStyle();

    QWidget* m_headerWidget;

    Core::CardWidget* m_messagesCard;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_scrollLayout;

    // Storage for message cards mapped by message ID
    std::map<uint32_t, Core::DbcMessageCard*> m_messageCards;
    QLabel* m_titleLabel;
    QPushButton* m_closeButton;
};

}  // namespace Logging