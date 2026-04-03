#include "math/service/variable_registry.hpp"

#include <cassert>

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
                               std::function<std::unique_ptr<IVariable>()> factory) -> IVariable*
{
    std::lock_guard lock(m_mutex);

    auto it = m_entries.find(configKey);
    if (it != m_entries.end())
    {
        ++it->second.refCount;
        return it->second.variable.get();
    }

    auto var = factory();
    auto* raw = var.get();
    m_entries[configKey] = Entry{std::move(var), 1};

    // If it's a CAN signal variable, register in the fast lookup map
    if (auto* canVar = dynamic_cast<CanSignalVariable*>(raw))
    {
        const std::string lookupKey =
            std::to_string(canVar->messageId()) + ":" + canVar->signalName();
        m_canSignalMap[lookupKey] = canVar;
    }

    return raw;
}

void VariableRegistry::release(const std::string& configKey)
{
    std::lock_guard lock(m_mutex);

    auto it = m_entries.find(configKey);
    if (it == m_entries.end()) return;

    assert(it->second.refCount > 0 && "release() called without matching acquire()");
    --it->second.refCount;
    if (it->second.refCount == 0)
    {
        // Clean up CAN signal map entry if applicable
        if (auto* canVar = dynamic_cast<CanSignalVariable*>(it->second.variable.get()))
        {
            const std::string lookupKey =
                std::to_string(canVar->messageId()) + ":" + canVar->signalName();
            m_canSignalMap.erase(lookupKey);
        }
        m_entries.erase(it);
    }
}

void VariableRegistry::updateAll() const
{
    std::lock_guard lock(m_mutex);

    for (const auto& [key, entry] : m_entries)
    {
        entry.variable->update();
    }
}

auto VariableRegistry::dbcConfig() const -> const Core::DbcConfig*
{
    std::lock_guard lock(m_mutex);
    return m_dbcConfig.has_value() ? &m_dbcConfig.value() : nullptr;
}

void VariableRegistry::onDbcParsed(const Core::DBCParsedEvent& event)
{
    std::lock_guard lock(m_mutex);
    m_dbcConfig = event.config;
    rebuildCanSignalMap();
}

void VariableRegistry::onCanDbcReceived(const Core::ReceivedCanDbcEvent& event)
{
    std::lock_guard lock(m_mutex);

    const auto& msg = event.canMessage;
    for (const auto& signal : msg.signalValues)
    {
        const std::string key = std::to_string(msg.messageId) + ":" + signal.name;
        const auto it = m_canSignalMap.find(key);
        if (it != m_canSignalMap.end())
        {
            it->second->setValue(signal.value);
        }
    }
}

void VariableRegistry::rebuildCanSignalMap()
{
    // Called with m_mutex already held
    m_canSignalMap.clear();
    for (const auto& [configKey, entry] : m_entries)
    {
        if (auto* canVar = dynamic_cast<CanSignalVariable*>(entry.variable.get()))
        {
            const std::string lookupKey =
                std::to_string(canVar->messageId()) + ":" + canVar->signalName();
            m_canSignalMap[lookupKey] = canVar;
        }
    }
}

}  // namespace Math
