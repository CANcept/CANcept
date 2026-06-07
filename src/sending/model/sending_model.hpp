/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QString>
#include <functional>
#include <mutex>
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
        Role_CycleIntervalUs,
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
        return m_cyclicState.intervalUs;
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
     * @brief Registers a callable that provides a signal's value dynamically (e.g. from a
     * ValueFunction). Called on every send cycle in repeated sending for up-to-date values.
     */
    using SignalEvaluator = std::function<double()>;
    void setSignalEvaluator(uint16_t messageId, const std::string& signalName,
                            SignalEvaluator evaluator);

    /** @brief Removes all evaluators (call before destroying widgets that evaluators reference). */
    void clearEvaluators();

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
    void forEachPendingMessage(const std::function<void(Core::RawCanMessage&)>& rawHandler,
                               const std::function<void(Core::DbcCanMessage&)>& dbcHandler) const;

    /**
     * @brief Builds the send cache for the current selection.
     */
    void buildSendCache();

    /**
     * @brief Iterates the pre-built message cache, updating signal values and invoking handlers.
     */
    void forEachCachedMessage(const std::function<void(Core::RawCanMessage&)>& rawHandler,
                              const std::function<void(Core::DbcCanMessage&)>& dbcHandler);

   private:
    /**
     * @brief Creates a unique key for a signal (messageId:signalName).
     */
    [[nodiscard]] static auto makeSignalKey(uint16_t messageId,
                                            const std::string& signalName) -> std::string;

    // Navigation & Mode
    Mode m_currentMode = Mode::Raw;

    /** @brief Pre-built messages reused each cycle */
    std::vector<Core::DbcCanMessage> m_messageCache;

    // Cyclic Transmission State
    struct CyclicState {
        bool isEnabled = false;  // The user "intent" to send cyclically
        int intervalUs = 1000;   // Default interval
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

    /** @brief Dynamic evaluators per signal (e.g. math expressions). Guarded by m_evalMutex. */
    mutable std::mutex m_evalMutex;
    std::map<std::string, SignalEvaluator> m_signalEvaluators;

    /** @brief Current DBC config (owned by this model) */
    std::optional<Core::DbcConfig> m_currentDbc;
};

}  // namespace Sending