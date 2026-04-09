#pragma once
#include <random>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/interface/i_fault_handler.hpp"
#include "fault_injector/types/Fault.hpp"

namespace FaultInjector {

/**
 * @brief Concrete implementation of Core::IFaultHandler.
 *
 * Raw faults operate at the byte level (bit flips, random corruption).
 * DBC faults operate at the decoded signal level (value overrides).
 *
 * Each fault evaluates its trigger chain first; only matching faults are
 * applied. Faults are applied in the order they were added.
 */
class FaultHandler final : public Core::IFaultHandler
{
   public:
    /**
     * @brief Constructs the FaultHandler with a fixed snapshot of faults.
     * @param rawFaults Faults applied at the raw byte level.
     * @param dbcFaults Faults applied at the decoded signal level.
     */
    FaultHandler(std::vector<RawFault> rawFaults, std::vector<DbcFault> dbcFaults)
        : m_rawFaults(std::move(rawFaults)), m_dbcFaults(std::move(dbcFaults))
    {
        m_random.seed(std::random_device()());
    }
    ~FaultHandler() override = default;

    /**
     * @brief Applies all matching raw faults to the given message in place.
     *
     * Each RawFault checks its trigger conditions against the message
     * and, if they match, applies its effects to the raw bytes.
     *
     * @param id the identifier
     * @param dlc the dlc
     * @param data the data of the message
     */
    void inject(uint16_t& id, uint8_t& dlc, std::array<char, 8>& data) override;

    /**
     * @brief Applies all matching DBC faults to the given message in place.
     *
     * Each DbcFault checks its trigger conditions against the decoded
     * signal values and, if they match, applies its effects.
     *
     * @param message The decoded CAN message to mutate.
     */
    void inject(Core::DbcCanMessage& message) override;

   private:
    std::vector<RawFault> m_rawFaults;
    std::vector<DbcFault> m_dbcFaults;
    std::mt19937 m_random;
};

}  // namespace FaultInjector
