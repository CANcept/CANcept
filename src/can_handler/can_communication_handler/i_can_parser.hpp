//
// Created by flori on 28.12.2025.
//

#ifndef CANBUSMANAGER_I_CAN_HANDLER_HPP
#define CANBUSMANAGER_I_CAN_HANDLER_HPP
#include <CanDriver.hpp>
using sockcanpp::CanMessage;
#include "core/interface/i_event_broker.hpp"

namespace CanHandler {
/**
 * @brief ICanParser is an abstract class that is the foundation for a can parser. If you inherit
 * this class, you need to implement the parseReceivedMessage method that parses a can message.
 * It provides a function you pass CAN message to, to send them to the connected can
 * device. It returns a bool indicating if the message was successfully sent.
 */
class ICanParser
{
   public:
    explicit ICanParser(Core::IEventBroker& eventBroker,
                        const std::function<bool(const CanMessage&)>& sendFunction)
        : broker(eventBroker), sendFunction(sendFunction)
    {
    }
    virtual ~ICanParser() = default;
    /**
     * @brief Virtual method, that parses a message received over a CAN bus.
     * @param canMessage The received message
     */
    virtual void parseReceivedMessage(const sockcanpp::CanMessage* canMessage);

   private:
    /**
     * @brief The event broker to send events to
     */
    Core::IEventBroker& broker;
    /**
     * @brief Function to send messages to for sending over the CAN Bus device. It returns a bool
     * indicating if the message was sent successfully
     */
    std::function<bool(const CanMessage&)> sendFunction;
};
}  // namespace CanHandler

#endif  // CANBUSMANAGER_I_CAN_HANDLER_HPP
