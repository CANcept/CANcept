#pragma once

#include <QAbstractItemModel>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"

namespace Sending {

class SendingModel final : public QAbstractItemModel
{
    Q_OBJECT

   public:
    enum class Mode { Raw = 0, Dbc = 1 };

    /**
     * @enum SendingRoles
     * @brief Custom roles for accessing CAN composition and cyclic timing data.
     */
    enum SendingRoles {
        Role_Value = Qt::UserRole + 1,
        Role_CanId,
        Role_SignalValue,
        Role_ActiveMode,
        Role_IsCyclicEnabled,
        Role_CycleIntervalMs,
        Role_IsCurrentlySending
    };

    explicit SendingModel(QObject* parent = nullptr);

    [[nodiscard]] auto index(int row, int column, const QModelIndex& parent = QModelIndex()) const
        -> QModelIndex override;
    [[nodiscard]] auto parent(const QModelIndex& child) const -> QModelIndex override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override;

    /**
     * @brief Returns data for UI display or logic checks.
     */
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

    /**
     * @brief Updates internal state (e.g., user changes the interval or toggles cyclic).
     */
    auto setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) -> bool override;

    [[nodiscard]] auto isCyclicEnabled() const -> bool
    {
        return m_cyclicState.isEnabled;
    }
    [[nodiscard]] auto cycleInterval() const -> int
    {
        return m_cyclicState.intervalMs;
    }
    [[nodiscard]] auto isCurrentlySending() const -> bool
    {
        return m_cyclicState.isSending;
    }

    void updateDbcConfig(const Core::DbcConfig& config);
    void setTransmissionStatus(bool isActive);

    /**
     * @brief Toggles message selection for transmission.
     */
    void setMessageSelected(uint16_t messageId, bool selected);

    /**
     * @brief Checks if a message is selected.
     */
    [[nodiscard]] auto isMessageSelected(uint16_t messageId) const -> bool;

    /**
     * @brief Toggles signal selection for transmission.
     * @param messageId The message ID that contains the signal
     * @param signalName The signal name (unique within the message)
     * @param selected Whether the signal is selected
     */
    void setSignalSelected(uint16_t messageId, const std::string& signalName, bool selected);

    /**
     * @brief Checks if a signal is selected.
     * @param messageId The message ID that contains the signal
     * @param signalName The signal name (unique within the message)
     */
    [[nodiscard]] auto isSignalSelected(uint16_t messageId,
                                        const std::string& signalName) const -> bool;

    /**
     * @brief Sets a signal's value.
     * @param messageId The message ID that contains the signal
     * @param signalName The signal name (unique within the message)
     * @param value The physical value
     */
    void setSignalValue(uint16_t messageId, const std::string& signalName, double value);

    /**
     * @brief Gets the current DBC config pointer.
     */
    [[nodiscard]] auto currentDbcConfig() const -> const Core::DbcConfig*
    {
        return m_currentDbc.has_value() ? &m_currentDbc.value() : nullptr;
    }

    /**
     * @brief Sets the raw CAN ID for transmission.
     */
    void setRawCanId(uint16_t canId);

    /**
     * @brief Sets the raw data bytes for transmission.
     */
    void setRawData(const std::vector<uint8_t>& data);

   signals:
    /** * @brief Emitted when the Model determines a Raw message should be sent.
     * Triggered by manual user action or the internal cyclic timer.
     */
    void requestSendRaw(const std::string& device, const Core::RawCanMessage& message);

    /** * @brief Emitted when the Model determines a DBC message should be sent.
     * Triggered by manual user action or the internal cyclic timer.
     */
    void requestSendDbc(const std::string& device, const Core::DbcCanMessage& message);

   public slots:
    /**
     * @brief Triggers the immediate transmission of the currently configured message.
     */
    void transmitCurrent();

   public:
    /**
     * @brief Builds pending messages and passes them to the provided handlers.
     */
    void forEachPendingMessage(
        const std::function<void(const Core::RawCanMessage&)>& rawHandler,
        const std::function<void(const Core::DbcCanMessage&)>& dbcHandler) const;

   private:
    /**
     * @brief Creates a unique key for a signal (messageId:signalName).
     */
    [[nodiscard]] static auto makeSignalKey(uint16_t messageId,
                                            const std::string& signalName) -> std::string;

    // Navigation & Mode
    Mode m_currentMode = Mode::Raw;

    // Cyclic Transmission State
    struct CyclicState {
        bool isEnabled = false;  // The user "intent" to send cyclically
        int intervalMs = 100;    // Default interval
        bool isSending = false;  // Whether the component is actively sending
    } m_cyclicState;

    // Payload States
    struct {
        uint16_t id = 0;
        std::vector<uint8_t> data = std::vector<uint8_t>(8, 0);
        uint8_t dlc = 0;
    } m_rawState;

    /** * @brief Stores current user-input values for signals.
     * Key: "messageId:signalName" (unique identifier)
     * Value: The physical value (double)
     */
    std::map<std::string, double> m_dynamicSignalValues;

    /** @brief Stores which signals are selected for transmission.
     * Key: "messageId:signalName" (unique identifier)
     */
    std::set<std::string> m_selectedSignalNames;

    /** @brief Current DBC config (owned by this model) */
    std::optional<Core::DbcConfig> m_currentDbc;
};

}  // namespace Sending