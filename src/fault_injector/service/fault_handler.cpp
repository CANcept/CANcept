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

#include "fault_injector/service/fault_handler.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>

#include "fault_effect_handler.hpp"
#include "fault_trigger_handler.hpp"

namespace FaultInjector {

void FaultHandler::inject(uint16_t& id, uint8_t& dlc, std::array<char, 8>& data)
{
    for (size_t i = 0; i < m_rawFaults.size(); ++i)
    {
        const auto& [trigger, effect, strategy, mutation] = m_rawFaults[i];

        if (!m_rawLatched[i])
        {
            const bool triggered = std::ranges::all_of(trigger, [&](const RawTrigger& t) {
                return firesRawTrigger(t, id, dlc, data, m_random);
            });
            if (!triggered) continue;

            if (std::holds_alternative<LatchMutation>(mutation)) m_rawLatched[i] = true;
        }

        std::ranges::for_each(
            effect, [&](const RawEffect& e) { applyRawEffect(e, id, dlc, data, m_random); });
        updateFrameStrategy(strategy);
    }
}

void FaultHandler::inject(Core::DbcCanMessage& message)
{
    std::unordered_map<std::string, Core::DbcCanSignal*> signalMap;
    signalMap.reserve(message.signalValues.size());
    for (auto& signal : message.signalValues)
    {
        signalMap.emplace(signal.name, &signal);
    }

    for (size_t i = 0; i < m_dbcFaults.size(); ++i)
    {
        const auto& [trigger, effect, strategy, mutation] = m_dbcFaults[i];

        if (!m_dbcLatched[i])
        {
            const bool triggered = std::ranges::all_of(trigger, [&](const DbcTrigger& t) {
                return firesDbcTrigger(t, signalMap, m_random);
            });
            if (!triggered) continue;

            if (std::holds_alternative<LatchMutation>(mutation)) m_dbcLatched[i] = true;
        }

        std::ranges::for_each(effect,
                              [&](const DbcEffect& e) { applyDbcEffect(e, signalMap, m_random); });
        updateFrameStrategy(strategy);
    }
}

void FaultHandler::updateFrameStrategy(const Strategy& strategy)
{
    if (m_frameDrop) return;

    std::visit(entt::overloaded{
                   [](const ImmediateStrategy&) {},
                   [this](const DelayedStrategy& s) {
                       m_frameMaxDelayUs = std::max(m_frameMaxDelayUs, s.delayUs);
                   },
                   [this](const DropStrategy&) {
                       m_frameDrop = true;
                       m_frameMaxDelayUs = 0;
                   },
               },
               strategy);
}

Core::IFaultHandler::FrameResult FaultHandler::evaluate()
{
    const bool drop = m_frameDrop;
    const auto delayOffset = std::chrono::microseconds(m_frameMaxDelayUs);
    m_frameDrop = false;
    m_frameMaxDelayUs = 0;
    return {.drop = drop, .delayOffset = delayOffset};
}

}  // namespace FaultInjector