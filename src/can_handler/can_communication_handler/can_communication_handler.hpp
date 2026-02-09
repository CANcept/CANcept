//
// Created by flori on 28.12.2025.
//
#pragma once
#include <list>

#include "can_dbc_handler.hpp"
#include "can_device_handler.hpp"
#include "can_raw_handler.hpp"
#include "core/interface/i_lifecycle.hpp"
#include "i_can_parser.hpp"
using sockcanpp::CanMessage;

namespace CanHandler {
/**
 * @brief Orchestrates CAN communication by managing multiple specialized CAN parsers.
 *
 * It handles the connection to the can interface via the libsockcan library.
 * For that it provides a method to send a message to the current can interface as well as having
 * can handlers for handling incoming messages over the bus.
 *
 * It inherits from Core::ILifecycle, allowing it to automatically respond to
 * system-wide start and stop events via the provided EventBroker.
 */
class CanCommunicationHandler final : public Core::ILifecycle
{
   public:
    explicit CanCommunicationHandler(Core::IEventBroker& event_broker)
        : ILifecycle(event_broker), deviceHandler(std::make_unique<CanDeviceHandler>(event_broker))
    {
        can_handlers.push_back(std::make_unique<CanDbcHandler>(
            event_broker, [this](const CanMessage& canMessage) -> bool {
                return deviceHandler->sendCanMessage(canMessage);
            }));
        can_handlers.push_back(std::make_unique<CanRawHandler>(
            event_broker, [this](const CanMessage& canMessage) -> bool {
                return deviceHandler->sendCanMessage(canMessage);
            }));
    };

   protected:
    /**
     * @brief Called automatically when the application publishes AppStartedEvent.
     */
    void onStart() override;
    /**
     * @brief Called automatically when the application publishes AppStoppedEvent.
     */
    void onStop() override;

    /**
     * @brief Lets all can parsers register settings.
     */
    void registerSettings(Core::ISettingsRegistry& registry) override;

   private:
    /**
     * @brief Method that gets called periodically to check on new can messages over the bus.
     * It distributes eventual new messages to the connected can handlers for further processing.
     */
    void checkCanDeviceForMessages() const;
    /**
     * @brief A list of connected can handlers that are responsible for processing the raw messages
     * sent over the bus to events usable for the can bus manager.
     */
    std::list<std::unique_ptr<ICanParser>> can_handlers;
    /**
     * @brief The CAN device handler that handles all events related to the actual CAN device
     */
    std::unique_ptr<CanDeviceHandler> deviceHandler;
    /**
     * @brief Flag indicating if the CanCommunicationHandler should check for new CAN messages
     * periodically
     */
    std::atomic<bool> _execute;

    std::thread message_check_thread;

    const int MILLISECONDS_BETWEEN_PARSE_ATTEMPTS = 100;
};
}  // namespace CanHandler
