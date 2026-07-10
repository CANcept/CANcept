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

#include <filesystem>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "manipulation/service/manipulation_serializer.hpp"

using namespace Manipulation;

namespace {
auto tempFilePath() -> std::filesystem::path
{
    return std::filesystem::temp_directory_path() /
           std::filesystem::path("canbusmanager_manipulation_serializer_test_" +
                                 std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
                                 ".json");
}

/**
 * @brief Builds one manipulation touching every trigger/effect/strategy/mutation variant, so
 * the round-trip test exercises the full tagged-union encoding.
 */
auto buildRawManipulation() -> RawManipulation
{
    return RawManipulation{
        .trigger = {RawTrigger(IDTrigger(0x123)), RawTrigger(DLCTrigger(8)),
                    RawTrigger(RandomTrigger(0.5F))},
        .strategy = RawStrategy(RawEffectStrategy{
            .effects = {RawEffect(BitFlipEffect(1, 2)), RawEffect(RandomBitFlipEffect())}}),
        .mutation = Mutation(LatchMutation()),
    };
}

auto buildDbcManipulationWithInsertMessage() -> DbcManipulation
{
    return DbcManipulation{
        .trigger = {DbcTrigger(SignalNameTrigger("EngineRPM")),
                    DbcTrigger(SignalThresholdTrigger("Speed", 100.0, true)),
                    DbcTrigger(RandomTrigger(0.25F))},
        .strategy = DbcStrategy(DbcInsertStrategy{
            .delayUs = 1500,
            .message =
                Core::DbcCanMessage{
                    .receiveTime = std::chrono::nanoseconds(42),
                    .signalValues = {Core::DbcCanSignal{.name = "EngineRPM", .value = 3000.0}},
                    .messageId = 256,
                },
        }),
        .mutation = Mutation(NoMutation()),
    };
}

auto buildDbcManipulationWithoutInsertMessage() -> DbcManipulation
{
    return DbcManipulation{
        .trigger = {DbcTrigger(SignalNameTrigger("Speed"))},
        .strategy =
            DbcStrategy(DbcEffectStrategy{.effects = {DbcEffect(ValueSetEffect("Speed", 42.0)),
                                                      DbcEffect(ClampEffect("Speed", 0.0, 200.0)),
                                                      DbcEffect(NoiseEffect("Speed", 1.5))}}),
        .mutation = Mutation(NoMutation()),
    };
}

auto buildRawDropAndDelay() -> std::vector<ManipulationEntry>
{
    return {
        ManipulationEntry(RawManipulation{
            .trigger = {},
            .strategy = RawStrategy(DropStrategy{}),
            .mutation = Mutation(NoMutation()),
        }),
        ManipulationEntry(RawManipulation{
            .trigger = {},
            .strategy = RawStrategy(DelayedStrategy(250)),
            .mutation = Mutation(NoMutation()),
        }),
        ManipulationEntry(DbcManipulation{
            .trigger = {},
            .strategy = DbcStrategy(DropStrategy{}),
            .mutation = Mutation(NoMutation()),
        }),
    };
}

}  // namespace

TEST(ManipulationSerializerTest, RoundTripsThroughFilePreservesEncodedShape)
{
    std::vector<ManipulationEntry> entries;
    entries.emplace_back(buildRawManipulation());
    entries.emplace_back(buildDbcManipulationWithInsertMessage());
    entries.emplace_back(buildDbcManipulationWithoutInsertMessage());
    for (auto& entry : buildRawDropAndDelay())
    {
        entries.push_back(std::move(entry));
    }

    const nlohmann::json originalJson = entries;

    const auto path = tempFilePath();
    ASSERT_TRUE(ManipulationSerializer::save(path, entries));

    const auto loaded = ManipulationSerializer::load(path);
    const nlohmann::json loadedJson = loaded;

    EXPECT_EQ(loadedJson, originalJson);

    std::filesystem::remove(path);
}

TEST(ManipulationSerializerTest, LoadFromMissingFileReturnsEmptyList)
{
    const auto loaded =
        ManipulationSerializer::load("/tmp/this_file_should_not_exist_123456789.json");
    EXPECT_TRUE(loaded.empty());
}
