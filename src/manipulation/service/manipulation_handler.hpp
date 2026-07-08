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
#include <string>
#include <unordered_map>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/interface/i_manipulation_handler.hpp"
#include "manipulation/types/manipulation.hpp"

namespace Manipulation {

/**
 * @brief Returns a fresh seed for a ManipulationHandler's RNG.
 *
 * Draws from one process-wide generator instead of std::random_device directly, since a
 * new ManipulationHandler is constructed on every send/evaluate call site, and querying
 * random_device that often can yield correlated seeds on platforms with a coarse or
 * low-entropy fallback implementation.
 */
inline auto nextManipulationSeed() -> std::mt19937::result_type
{
    static std::mt19937 seedSource{std::random_device{}()};
    return seedSource();
}

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
        m_random.seed(nextManipulationSeed());
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
     * @brief Applies a fired raw manipulation's strategy.
     *
     * For an effect strategy, applies its effects to the raw bytes. For delayed/drop
     * strategies, updates the per-frame state (priority: drop wins over everything;
     * among delayed, the maximum delay wins).
     *
     * @param strategy The strategy of the manipulation that just fired.
     * @param id the identifier
     * @param dlc the dlc
     * @param data the data of the message
     */
    void applyRawStrategy(const RawStrategy& strategy, uint16_t& id, uint8_t& dlc,
                          std::array<char, 8>& data);

    /**
     * @brief Applies a fired DBC manipulation's strategy.
     *
     * For an effect strategy, applies its effects to the decoded signal values. For
     * delayed/drop strategies, updates the per-frame state (priority: drop wins over
     * everything; among delayed, the maximum delay wins).
     *
     * @param strategy The strategy of the manipulation that just fired.
     * @param signalMap Decoded signals of the message, keyed by name.
     * @param message The decoded message being processed, used verbatim as the inserted
     * message when the insert strategy is configured to copy the triggering frame.
     */
    void applyDbcStrategy(const DbcStrategy& strategy,
                          std::unordered_map<std::string, Core::DbcCanSignal*>& signalMap,
                          const Core::DbcCanMessage& message);

    std::vector<RawManipulation> m_rawManipulations;
    std::vector<bool> m_rawLatched;
    std::vector<DbcManipulation> m_dbcManipulations;
    std::vector<bool> m_dbcLatched;
    std::mt19937 m_random;

    /** @brief Per-frame drop flag, set by applyRawStrategy()/applyDbcStrategy(), cleared by
     * evaluate(). */
    bool m_frameDrop{false};
    /** @brief Per-frame maximum requested delay in microseconds, cleared by evaluate(). */
    uint32_t m_frameMaxDelayUs{0};
    /** @brief Per-frame pending insertions, cleared by evaluate(). */
    std::vector<Core::IManipulationHandler::PendingInsertion> m_frameInsertions;
};

}  // namespace Manipulation
