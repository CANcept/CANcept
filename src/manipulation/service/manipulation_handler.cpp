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

#include "manipulation/service/manipulation_handler.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>

#include "manipulation_effect_handler.hpp"
#include "manipulation_trigger_handler.hpp"

namespace Manipulation {

void ManipulationHandler::inject(uint16_t& id, uint8_t& dlc, std::array<char, 8>& data)
{
    for (size_t i = 0; i < m_rawManipulations.size(); ++i)
    {
        const auto& [trigger, strategy, mutation] = m_rawManipulations[i];

        if (!m_rawLatched[i])
        {
            const bool triggered = std::ranges::all_of(trigger, [&](const RawTrigger& t) {
                return firesRawTrigger(t, id, dlc, data, m_random);
            });
            if (!triggered) continue;

            if (std::holds_alternative<LatchMutation>(mutation)) m_rawLatched[i] = true;
        }

        applyRawStrategy(strategy, id, dlc, data);
    }
}

void ManipulationHandler::inject(Core::DbcCanMessage& message)
{
    std::unordered_map<std::string, Core::DbcCanSignal*> signalMap;
    signalMap.reserve(message.signalValues.size());
    for (auto& signal : message.signalValues)
    {
        signalMap.emplace(signal.name, &signal);
    }

    for (size_t i = 0; i < m_dbcManipulations.size(); ++i)
    {
        const auto& [trigger, strategy, mutation] = m_dbcManipulations[i];

        if (!m_dbcLatched[i])
        {
            const bool triggered = std::ranges::all_of(trigger, [&](const DbcTrigger& t) {
                return firesDbcTrigger(t, signalMap, m_random);
            });
            if (!triggered) continue;

            if (std::holds_alternative<LatchMutation>(mutation)) m_dbcLatched[i] = true;
        }

        applyDbcStrategy(strategy, signalMap, message);
    }
}

void ManipulationHandler::applyRawStrategy(const RawStrategy& strategy, uint16_t& id, uint8_t& dlc,
                                           std::array<char, 8>& data)
{
    std::visit(entt::overloaded{
                   [&](const RawEffectStrategy& s) {
                       std::ranges::for_each(s.effects, [&](const RawEffect& e) {
                           applyRawEffect(e, id, dlc, data, m_random);
                       });
                   },
                   [this](const DelayedStrategy& s) {
                       if (m_frameDrop) return;
                       m_frameMaxDelayUs = std::max(m_frameMaxDelayUs, s.delayUs);
                   },
                   [this](const DropStrategy&) {
                       m_frameDrop = true;
                       m_frameMaxDelayUs = 0;
                   },
               },
               strategy);
}

void ManipulationHandler::applyDbcStrategy(
    const DbcStrategy& strategy, std::unordered_map<std::string, Core::DbcCanSignal*>& signalMap,
    const Core::DbcCanMessage& message)
{
    std::visit(entt::overloaded{
                   [&](const DbcEffectStrategy& s) {
                       std::ranges::for_each(s.effects, [&](const DbcEffect& e) {
                           applyDbcEffect(e, signalMap, m_random);
                       });
                   },
                   [this](const DelayedStrategy& s) {
                       if (m_frameDrop) return;
                       m_frameMaxDelayUs = std::max(m_frameMaxDelayUs, s.delayUs);
                   },
                   [this](const DropStrategy&) {
                       m_frameDrop = true;
                       m_frameMaxDelayUs = 0;
                   },
                   [this, &message](const DbcInsertStrategy& s) {
                       m_frameInsertions.push_back({std::chrono::microseconds(s.delayUs),
                                                    s.message ? *s.message : message});
                   },
               },
               strategy);
}

Core::IManipulationHandler::FrameResult ManipulationHandler::evaluate()
{
    const bool drop = m_frameDrop;
    const auto delayOffset = std::chrono::microseconds(m_frameMaxDelayUs);
    auto insertions = std::move(m_frameInsertions);
    m_frameDrop = false;
    m_frameMaxDelayUs = 0;
    return {.drop = drop, .delayOffset = delayOffset, .insertions = std::move(insertions)};
}

}  // namespace Manipulation