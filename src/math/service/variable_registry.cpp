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

#include "math/service/variable_registry.hpp"

#include <cassert>
#include <mutex>
#include <ranges>

#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "math/service/variables/can_signal_variable.hpp"

namespace Math {

VariableRegistry::VariableRegistry(Core::IEventBroker& broker) : ILifecycle(broker) {}

void VariableRegistry::onStart()
{
    m_dbcParsedConn = m_eventBroker.subscribe<Core::DBCParsedEvent>(
        [this](const Core::DBCParsedEvent& event) { onDbcParsed(event); });

    m_canDbcReceivedConn = m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
        [this](const Core::ReceivedCanDbcEvent& event) { onCanDbcReceived(event); });
}

void VariableRegistry::onStop()
{
    m_dbcParsedConn.release();
    m_canDbcReceivedConn.release();
}

auto VariableRegistry::acquire(const std::string& configKey,
                               const std::function<std::unique_ptr<IVariable>()>& factory)
    -> IVariable*
{
    std::unique_lock lock(m_mutex);  // Blocks all readers and writers

    if (const auto it = m_entries.find(configKey); it != m_entries.end())
    {
        ++it->second.refCount;
        return it->second.variable.get();
    }

    auto var = factory();
    auto* raw = var.get();
    m_entries[configKey] = Entry{std::move(var), 1};

    rebuildCanSignalMap();
    return raw;
}

void VariableRegistry::release(const std::string& configKey)
{
    std::unique_lock lock(m_mutex);

    auto it = m_entries.find(configKey);
    if (it == m_entries.end()) return;

    assert(it->second.refCount > 0);
    --it->second.refCount;

    if (it->second.refCount == 0)
    {
        m_entries.erase(it);
        rebuildCanSignalMap();  // Refresh fast map after removal
    }
}

auto VariableRegistry::dbcConfig() const -> const Core::DbcConfig*
{
    std::shared_lock lock(m_mutex);
    return m_dbcConfig.has_value() ? &m_dbcConfig.value() : nullptr;
}

void VariableRegistry::reset()
{
    std::unique_lock lock(m_mutex);
    for (const auto& entry : m_entries | std::views::values)
    {
        entry.variable->reset();
    }
}

void VariableRegistry::updateAll() const
{
    std::shared_lock lock(m_mutex);  // Parallel friendly
    for (const auto& entry : m_entries | std::views::values)
    {
        entry.variable->update();
    }
}

void VariableRegistry::onDbcParsed(const Core::DBCParsedEvent& event)
{
    std::unique_lock lock(m_mutex);  // Writer lock for structural change
    m_dbcConfig = event.config;
    rebuildCanSignalMap();
}

void VariableRegistry::onCanDbcReceived(const Core::ReceivedCanDbcEvent& event)
{
    std::shared_lock lock(m_mutex);
    const auto& msg = event.canMessage;

    const auto it = m_canSignalMap.find(msg.messageId);
    if (it == m_canSignalMap.end()) return;

    for (auto* var : it->second)
    {
        for (const auto& [name, value] : msg.signalValues)
        {
            if (name == var->signalName())
            {
                var->setValue(value);  // This call is atomic
                break;
            }
        }
    }
}

void VariableRegistry::rebuildCanSignalMap()
{
    m_canSignalMap.clear();

    for (const auto& [variable, refCount] : m_entries | std::views::values)
    {
        if (auto* canVar = dynamic_cast<CanSignalVariable*>(variable.get()))
        {
            m_canSignalMap[canVar->messageId()].push_back(canVar);
        }
    }
}

}  // namespace Math
