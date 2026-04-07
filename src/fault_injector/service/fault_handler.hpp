#pragma once
#include <random>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "fault_injector/types/Fault.hpp"

namespace FaultInjector {

/**
 * @brief Applies registered faults to outgoing CAN messages.
 *
 * Holds two independent fault lists — one for raw bit-level injection
 * and one for decoded signal-level injection.
 */
class FaultHandler
{
   public:
    /**
     * @brief Constructs the FaultHandler with a fixed set of faults.
     * @param rawFaults Faults applied at the raw byte level.
     * @param dbcFaults Faults applied at the decoded signal level.
     */
    FaultHandler(std::vector<RawFault> rawFaults, std::vector<DbcFault> dbcFaults)
        : m_rawFaults(std::move(rawFaults)), m_dbcFaults(std::move(dbcFaults))
    {
        m_random.seed(std::random_device()());
    }
    ~FaultHandler() = default;

    /**
     * @brief Applies all matching raw faults to the given message in place.
     * @param message The raw CAN message to mutate.
     */
    void inject(Core::RawCanMessage &message);

    /**
     * @brief Applies all matching DBC faults to the given message in place.
     * @param message The decoded CAN message to mutate.
     */
    void inject(Core::DbcCanMessage &message);

   private:
    std::vector<RawFault> m_rawFaults;
    std::vector<DbcFault> m_dbcFaults;
    std::mt19937 m_random;
};

}  // namespace FaultInjector
