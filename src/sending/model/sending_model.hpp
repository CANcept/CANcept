#pragma once

#include <QAbstractItemModel>
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

   private slots:
    /**
     * @brief Internal slot handling the cyclic timer timeout.
     * Calls transmitCurrent() automatically.
     */
    void onCyclicTimerTimeout();

   private:
    /**
     * @brief internal helper to start/stop QTimer based on m_cyclicState.
     */
    void updateTimerState();

    // Navigation & Mode
    Mode m_currentMode = Mode::Raw;
    // Cyclic Transmission State
    struct CyclicState {
        bool isEnabled = false;  // The user "intent" to send cyclically
        int intervalMs = 100;    // Default interval
        bool isSending = false;  // The actual "live" state of the transmission
    } m_cyclicState;

    // Payload States
    struct {
        uint32_t id = 0;
        std::vector<uint8_t> data = std::vector<uint8_t>(8, 0);
    } m_rawState;

    /** * @brief Stores current user-input values for signals.
     * Key: Signal name or unique ID
     * Value: The physical value (double)
     */
    std::map<std::string, double> m_dynamicSignalValues;

    /** @brief Stores which messages are selected for transmission (checkbox state) */
    std::vector<uint32_t> m_selectedMessageIds;

    Core::DbcConfig* m_currentDbc;

    // Moved from SendingDelegate: The Model now owns the timing source of truth
    QTimer* m_cyclicTimer;
};

}  // namespace Sending