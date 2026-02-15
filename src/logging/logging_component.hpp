#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
#include <memory>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"
#include "core/event/can_event.hpp"
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"
#include "logging/delegate/logging_delegate.hpp"
#include "logging/logger.hpp"
#include "logging/model/logging_model.hpp"
#include "logging/view/logging_view.hpp"
#include "logging/view/message_selection_dialog.hpp"

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

    /** @brief Bridge signal to forward raw CAN frames to the model */
    void receiveRawFrame(const Core::RawCanMessage& message);

    /** @brief Bridge signal to forward decoded DBC messages to the model */
    void receiveDbcSignals(const Core::DbcCanMessage& message);

   private slots:
    void startLogging();
    void stopLogging();
    void exportLogSession(const QString& sessionId, const QString& filePath);
    void onDetailRequested(const QModelIndex& index);

   private:
    /**
     * @brief Writes session metadata to CSV.
     * @note Message-level export can be added once the model stores frame data.
     */
    bool writeToCsv(const QString& sessionId, const QString& filePath);

    /** @brief Builds a detail widget for a specific session */
    QWidget* createDetailWidget(const LogSession* session);

   private:
    std::unique_ptr<LoggingModel> m_model;
    std::unique_ptr<LoggingView> m_view;
    std::unique_ptr<LoggingDelegate> m_delegate;
    std::unique_ptr<MessageSelectionDialog> m_selectionDialog;

    QTimer* m_timer;
    QElapsedTimer m_elapsedTimer;

    /** @brief RAII handles for broker subscriptions */
    Core::Connection m_rawMsgConn;
    Core::Connection m_dbcMsgConn;
    Core::Connection m_parseSuccessConn;
    Core::Connection m_parseErrorConn;

    /** @brief Tracks whether logging is DBC-based (true) or raw (false) */
    bool m_isDbcLogging{true};
};

}  // namespace Logging
