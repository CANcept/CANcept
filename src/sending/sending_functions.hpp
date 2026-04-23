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

#include "core/event/can_event.hpp"
#include "core/interface/i_event_broker.hpp"

namespace Sending {

/** @brief Context for every raw CAN frame queued for sending. */
struct RawSendContext {
    Core::IEventBroker* broker;
    Core::RawCanMessage message;
};

/** @brief Publishes the queued raw CAN frame to the bus. Used as the universal send callback. */
static void rawSendImpl(void* context)
{
    const auto* c = static_cast<RawSendContext*>(context);
    c->broker->publish(Core::SendCanMessageRawEvent(c->message));
}

}  // namespace Sending