//
// Created by flori on 28.12.2025.
//

#ifndef CANBUSMANAGER_CAN_COMMUNICATION_HANDLER_HPP
#define CANBUSMANAGER_CAN_COMMUNICATION_HANDLER_HPP
#include <list>

#include "can_dbc_handler.hpp"
#include "can_device_handler.hpp"
#include "can_raw_handler.hpp"
#include "core/interface/i_lifecycle.hpp"
#include "i_can_parser.hpp"

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
        : ILifecycle(event_broker),
          deviceHandler(CanDeviceHandler(event_broker)){
              /*
              can_handlers.push_back(CanDbcHandler(event_broker, [this](const CanMessage&
              canMessage) -> bool { return deviceHandler.sendCanMessage(&canMessage);
              }));
              can_handlers.push_back(CanRawHandler(event_broker, [this](const CanMessage&
              canMessage) -> bool { return deviceHandler.sendCanMessage(&canMessage);
              }));
              */
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

   private:
    /**
     * @brief Method that gets called periodically to check on new can messages over the bus.
     * It distributes eventual new messages to the connected can handlers for further processing.
     */
    void checkCanDeviceForMessages();
    /**
     * @brief A list of connected can handlers that are responsible for processing the raw messages
     * sent over the bus to events usable for the can bus manager.
     */
    std::list<ICanParser> can_handlers;
    /**
     * @brief The CAN device handler that handles all events related to the actual CAN device
     */
    CanDeviceHandler deviceHandler;
};
}  // namespace CanHandler

#endif  // CANBUSMANAGER_CAN_COMMUNICATION_HANDLER_HPP
