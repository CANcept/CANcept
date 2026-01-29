#pragma once

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>

namespace Core {

class DbcSignalRowWidget;  // Forward declaration
class StyledCheckBox;      // Forward declaration

/**
 * @class DbcMessageCard
 * @brief Reusable UI component for displaying a DBC message and its signals.
 *
 * Generic card widget that can be used across different modules:
 * - Sending: Message composition with checkbox for selection
 * - Logging: Message filtering with checkbox for selection
 * - Monitoring: Message display (checkbox can be hidden)
 *
 * The checkbox can be optionally hidden for read-only displays.
 */
class DbcMessageCard : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Configuration for message card appearance and behavior.
     */
    struct Config {
        bool showCheckbox = true;      ///< Show selection checkbox in header
        bool startExpanded = false;    ///< Start with signals visible
        QString checkboxTooltip = "";  ///< Tooltip for checkbox (context-specific)
    };

    /**
     * @brief Constructs a message card with custom configuration.
     * @param name Message name (e.g., "EngineData")
     * @param id CAN message ID (e.g., 0x100)
     * @param signalCount Number of signals in this message
     * @param config Configuration for card behavior
     * @param parent Parent widget
     */
    explicit DbcMessageCard(const QString& name, uint32_t id, int signalCount, const Config& config,
                            QWidget* parent = nullptr);

    /**
     * @brief Constructs a message card with default configuration.
     * @param name Message name (e.g., "EngineData")
     * @param id CAN message ID (e.g., 0x100)
     * @param signalCount Number of signals in this message
     * @param parent Parent widget
     */
    explicit DbcMessageCard(const QString& name, uint32_t id, int signalCount,
                            QWidget* parent = nullptr);

    ~DbcMessageCard() override = default;

    /** @brief Adds a signal row widget to the card body. */
    void addSignalRow(QWidget* rowWidget);

    /** @brief Clears all signal rows from the card body. */
    void clearSignalRows();

    // --- Accessors ---
    [[nodiscard]] StyledCheckBox* headerCheckbox() const
    {
        return m_headerCheckbox;
    }

    [[nodiscard]] bool isExpanded() const
    {
        return m_bodyContainer && m_bodyContainer->isVisible();
    }

    void setHeaderChecked(bool checked);
    void setExpanded(bool expanded);

   private:
    void setupUi(const QString& name, uint32_t id, int signalCount, const Config& config);

    QLabel* m_nameLabel;
    QLabel* m_idLabel;
    StyledCheckBox* m_headerCheckbox;
    QPushButton* m_expandBtn;

    QWidget* m_bodyContainer;
    QVBoxLayout* m_signalsLayout;
};

}  // namespace Core