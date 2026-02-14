#pragma once

#include <spdlog/spdlog.h>

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"
#include "core/event/can_event.hpp"
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"
#include "core/util/log_service.hpp"
#include "logging/delegate/logging_delegate.hpp"
#include "logging/model/logging_model.hpp"
#include "logging/view/logging_view.hpp"
#include "view/components/message_selection_dialog.hpp"

namespace Logging {

/**
 * @class LoggingComponent
 * @brief Controller / Composition Root for the Logging module.
 *
 * Responsibilities:
 * - Owns Model, View, and Delegate
 * - Wires View ↔ Model ↔ EventBroker
 * - Manages logging lifecycle and export
 */
class LoggingComponent final : public Core::ITabComponent
{
    Q_OBJECT

   public:
    explicit LoggingComponent(Core::IEventBroker& broker);
    ~LoggingComponent() override;

    auto getView() -> QWidget* override;

    void onStart() override;
    void onStop() override;

   signals:
    /**
     * @brief Emitted when a new DBC configuration is available.
     * Connected to the LoggingModel.
     */
    void dbcConfigurationChanged(const Core::DbcConfig& config);

   private slots:
    void startLogging();
    void stopLogging();
    void exportLogSession(const QString& sessionId, const QString& filePath);
    void onDetailRequested(const QModelIndex& index);

   private:
    /** @brief Builds a detail widget for a specific session */
    QWidget* createDetailWidget(const LogSession* session);

   private:
    std::unique_ptr<LoggingModel> m_model;
    std::unique_ptr<LoggingView> m_view;
    std::unique_ptr<LoggingDelegate> m_delegate;
    std::unique_ptr<MessageSelectionDialog> m_selectionDialog;

    QTimer* m_timer;
    QElapsedTimer m_elapsedTimer;

    /** @brief Session-specific logger for CAN data */
    std::shared_ptr<spdlog::logger> m_sessionLogger;
    std::string m_currentSessionId;

    /** @brief Thread-safe caches for event handlers (accessed from CAN thread) */
    std::atomic<bool> m_isRecording{false};
    std::map<uint32_t, std::vector<std::string>>
        m_selectedSignalsCache;                                     // messageId -> signal names
    std::unordered_map<uint32_t, std::string> m_messageNamesCache;  // messageId -> message name
    std::unordered_map<uint64_t, std::string>
        m_signalUnitsCache;   // hash(messageId,signalName) -> unit
    std::mutex m_cacheMutex;  // Protects caches during session start/stop

    /** @brief RAII handles for broker subscriptions */
    Core::Connection m_rawMsgConn;
    Core::Connection m_dbcMsgConn;
    Core::Connection m_parseSuccessConn;
    Core::Connection m_parseErrorConn;
};

}  // namespace Logging
