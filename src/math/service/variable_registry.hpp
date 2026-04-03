#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "core/dto/dbc_dto.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/interface/i_lifecycle.hpp"
#include "math/types/variables/i_variable.hpp"

namespace Math {

class CanSignalVariable;

/**
 * @brief Global registry owning deduplicated variable instances with reference counting.
 *
 * Variables are identified by config keys. Models acquire/release references.
 * The registry is the origin of truth for variable values; models own symbol mappings.
 */
class VariableRegistry final : public Core::ILifecycle
{
   public:
    explicit VariableRegistry(Core::IEventBroker& broker);
    ~VariableRegistry() override = default;

    /**
     * @brief Acquires a reference to a variable by config key.
     *
     * If a variable with this key already exists, increments its refcount and returns it.
     * Otherwise, creates it via the factory, sets refcount to 1, and returns it.
     */
    auto acquire(const std::string& configKey,
                 std::function<std::unique_ptr<IVariable>()> factory) -> IVariable*;

    /**
     * @brief Decrements the refcount for the given config key.
     *
     * If the refcount reaches 0, the variable is removed from the registry.
     */
    void release(const std::string& configKey);

    /**
     * @brief Calls update() on every registered variable before an evaluation cycle.
     */
    void updateAll() const;

    /**
     * @brief Returns the currently loaded DBC config, or nullptr if none loaded.
     */
    [[nodiscard]] auto dbcConfig() const -> const Core::DbcConfig*;

   protected:
    void onStart() override;
    void onStop() override;

   private:
    struct Entry {
        std::unique_ptr<IVariable> variable;
        int refCount = 0;
    };

    void onDbcParsed(const Core::DBCParsedEvent& event);
    void onCanDbcReceived(const Core::ReceivedCanDbcEvent& event);
    void rebuildCanSignalMap();

    mutable std::recursive_mutex m_mutex;
    std::unordered_map<std::string, Entry> m_entries;
    std::unordered_map<std::string, CanSignalVariable*> m_canSignalMap;
    std::optional<Core::DbcConfig> m_dbcConfig;

    Core::Connection m_dbcParsedConn;
    Core::Connection m_canDbcReceivedConn;
};

}  // namespace Math
