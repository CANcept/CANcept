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
#include <random>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/interface/i_fault_handler.hpp"
#include "fault_injector/types/fault.hpp"

namespace FaultInjector {

/**
 * @brief Concrete implementation of Core::IFaultHandler.
 *
 * Raw faults operate at the byte level (bit flips, random corruption).
 * DBC faults operate at the decoded signal level (value overrides).
 *
 * Each fault evaluates its trigger chain first; only matching faults are
 * applied. Faults are applied in the order they were added.
 *
 * For now mutations are hardcoded with more in the future they will get dynamically executed.
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
        : m_rawFaults(std::move(rawFaults)),
          m_rawLatched(m_rawFaults.size(), false),
          m_dbcFaults(std::move(dbcFaults)),
          m_dbcLatched(m_dbcFaults.size(), false)
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
    std::vector<bool> m_rawLatched;
    std::vector<DbcFault> m_dbcFaults;
    std::vector<bool> m_dbcLatched;
    std::mt19937 m_random;
};

}  // namespace FaultInjector
