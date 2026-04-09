#include "fault_injector/service/fault_handler.hpp"

#include <algorithm>

#include "fault_effect_handler.hpp"
#include "fault_trigger_handler.hpp"

namespace FaultInjector {

void FaultHandler::inject(uint16_t& id, uint8_t& dlc, std::array<char, 8>& data)
{
    for (const auto& fault : m_rawFaults)
    {
        const bool triggered = std::ranges::all_of(fault.trigger, [&](const RawTrigger& trigger) {
            return firesRawTrigger(trigger, id, dlc, data, m_random);
        });
        if (!triggered) continue;

        std::ranges::for_each(fault.effect, [&](const RawEffect& effect) {
            applyRawEffect(effect, id, dlc, data, m_random);
        });
    }
}

void FaultHandler::inject(Core::DbcCanMessage& message)
{
    for (const auto& fault : m_dbcFaults)
    {
        const bool triggered = std::ranges::all_of(fault.trigger, [&](const DbcTrigger& trigger) {
            return firesDbcTrigger(trigger, message, m_random);
        });
        if (!triggered) continue;

        std::ranges::for_each(fault.effect, [&](const DbcEffect& effect) {
            applyDbcEffect(effect, message, m_random);
        });
    }
}

}  // namespace FaultInjector