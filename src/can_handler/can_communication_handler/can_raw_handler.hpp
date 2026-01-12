//
// Created by flori on 28.12.2025.
//

#ifndef CANBUSMANAGER_CAN_RAW_HANDLER_HPP
#define CANBUSMANAGER_CAN_RAW_HANDLER_HPP
#include "core/event/can_event.hpp"
#include "i_can_parser.hpp"

namespace CanHandler {
/**
 * @brief Handles all events based on raw CAN messages.
 * It provides two main functionalities:
 * It receives CAN messages from the CanCommunicationHandler, which it publishes in raw form to the
 * event handler. Equally, it listens to send raw CAN message events and then sends them to the CAN
 * device via the CanCommunicationHandler.
 */
class CanRawHandler final : public ICanParser
{
   public:
    explicit CanRawHandler(Core::IEventBroker& eventBroker,
                           const std::function<bool(const CanMessage&)>& sendFunction)
        : ICanParser(eventBroker, sendFunction)
    {
        rawSendEventConnection = eventBroker.subscribe<Core::SendCanMessageRawEvent>(
            [this](const Core::SendCanMessageRawEvent& event) -> void {
                handleSendMessage(event);
            });
    };
    ~CanRawHandler() override = default;
    /**
     * @brief Publishes the received CAN message in its raw form to the event handler.
     * @param canMessage The received CAN message
     */
    void parseReceivedMessage(const sockcanpp::CanMessage* canMessage) override;
    /**
     * @brief Handles @code SendRawCanMessageEvent by sending the encoded CAN message to the CAN
     * device via the CanCommunicationHandler
     * @param event The sending event
     */
    void handleSendMessage(const Core::SendCanMessageRawEvent& event);

   private:
    /**
     * @brief A connection containing the subscription to sendRawCanMessage events
     */
    Core::Connection rawSendEventConnection;
};
}  // namespace CanHandler

#endif  // CANBUSMANAGER_CAN_RAW_HANDLER_HPP
