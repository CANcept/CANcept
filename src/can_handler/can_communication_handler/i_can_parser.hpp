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
#include <CanDriver.hpp>
#include <chrono>
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
    virtual ~ICanParser() {};
    /**
     * @brief Virtual method, that parses a message received over a CAN bus.
     * @param canMessage The received message
     */
    virtual void parseReceivedMessage(const CanMessage* canMessage,
                                      std::chrono::nanoseconds timestamp) {};

   protected:
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
