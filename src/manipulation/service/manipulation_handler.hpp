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
#include "core/interface/i_manipulation_handler.hpp"
#include "manipulation/types/manipulation.hpp"

namespace Manipulation {

/**
 * @brief Concrete implementation of Core::IManipulationHandler.
 *
 * Raw manipulations operate at the byte level (bit flips, random corruption).
 * DBC manipulations operate at the decoded signal level (value overrides).
 *
 * Each manipulation evaluates its trigger chain first; only matching manipulations are
 * applied. Manipulations are applied in the order they were added.
 *
 * For now mutations are hardcoded with more in the future they will get dynamically executed.
 */
class ManipulationHandler final : public Core::IManipulationHandler
{
   public:
    /**
     * @brief Constructs the ManipulationHandler with a fixed snapshot of manipulations.
     * @param rawManipulations Manipulations applied at the raw byte level.
     * @param dbcManipulations Manipulations applied at the decoded signal level.
     */
    ManipulationHandler(std::vector<RawManipulation> rawManipulations,
                        std::vector<DbcManipulation> dbcManipulations)
        : m_rawManipulations(std::move(rawManipulations)),
          m_rawLatched(m_rawManipulations.size(), false),
          m_dbcManipulations(std::move(dbcManipulations)),
          m_dbcLatched(m_dbcManipulations.size(), false)
    {
        m_random.seed(std::random_device()());
    }
    ~ManipulationHandler() override = default;

    /**
     * @brief Applies all matching raw manipulations to the given message in place.
     *
     * Each RawManipulation checks its trigger conditions against the message
     * and, if they match, applies its effects to the raw bytes.
     *
     * @param id the identifier
     * @param dlc the dlc
     * @param data the data of the message
     */
    void inject(uint16_t& id, uint8_t& dlc, std::array<char, 8>& data) override;

    /**
     * @brief Applies all matching DBC manipulations to the given message in place.
     *
     * Each DbcManipulation checks its trigger conditions against the decoded
     * signal values and, if they match, applies its effects.
     *
     * @param message The decoded CAN message to mutate.
     */
    void inject(Core::DbcCanMessage& message) override;

    /**
     * @brief Aggregates per-frame strategies and resets per-frame state.
     *
     * @return FrameResult with drop flag and maximum delay offset.
     */
    FrameResult evaluate() override;

   private:
    /**
     * @brief Updates per-frame strategy state when a manipulation fires.
     *
     * Priority: Drop wins over everything; among delayed, the maximum delay wins.
     *
     * @param strategy The strategy of the manipulation that just fired.
     */
    void updateFrameStrategy(const Strategy& strategy);

    std::vector<RawManipulation> m_rawManipulations;
    std::vector<bool> m_rawLatched;
    std::vector<DbcManipulation> m_dbcManipulations;
    std::vector<bool> m_dbcLatched;
    std::mt19937 m_random;

    /** @brief Per-frame drop flag, set by updateFrameStrategy(), cleared by evaluate(). */
    bool m_frameDrop{false};
    /** @brief Per-frame maximum requested delay in microseconds, cleared by evaluate(). */
    uint16_t m_frameMaxDelayUs{0};
};

}  // namespace Manipulation
