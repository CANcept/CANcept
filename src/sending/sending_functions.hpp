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

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "core/event/can_event.hpp"
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_manipulation_handler.hpp"
#include "worker/scheduled_item.hpp"

namespace Sending {

/** @brief Context for every raw CAN frame queued for sending. */
struct RawSendContext {
    Core::IEventBroker* broker;
    Core::RawCanMessage message;
    std::function<void()> onSent;
};

/** @brief Publishes the queued raw CAN frame to the bus. Used as the universal send callback. */
static void rawSendImpl(void* context)
{
    const auto* c = static_cast<RawSendContext*>(context);

    c->broker->publish(Core::SendCanMessageRawEvent(c->message));
    if (c->onSent)
    {
        c->onSent();
    }
}

/**
 * @brief Encodes each pending insertion and builds a ScheduledItem for it.
 *
 * Reuses the same encode-then-schedule pattern every other send path already follows, so
 * an inserted message is indistinguishable from any other frame once it reaches the queue.
 *
 * @param insertions Pending insertions accumulated by a ManipulationHandler this cycle.
 * @param broker Event broker used to encode the DBC message and later publish it.
 * @param base Reference time the insertion's delay is relative to.
 */
inline auto buildInsertionItems(
    const std::vector<Core::IManipulationHandler::PendingInsertion>& insertions,
    Core::IEventBroker& broker, const Clock::time_point base) -> std::vector<ScheduledItem>
{
    std::vector<ScheduledItem> items;
    items.reserve(insertions.size());
    for (const auto& insertion : insertions)
    {
        Core::DbcCanMessage mutableMessage = insertion.message;
        Core::RawCanMessage encoded;
        broker.publish(Core::EncodeCanMessageDbcEvent(mutableMessage, encoded));
        items.push_back(ScheduledItem{.scheduledAt = base + insertion.delayOffset,
                                      .onSend = &rawSendImpl,
                                      .context = std::make_shared<RawSendContext>(
                                          RawSendContext{.broker = &broker, .message = encoded})});
    }
    return items;
}

}  // namespace Sending