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

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "manipulation/types/manipulation.hpp"

// Leaf trigger/effect/strategy types only expose validating constructors (no default
// constructor), so nlohmann's default get<T>() machinery (which needs T{}) can't round-trip
// them. Each gets a full nlohmann::adl_serializer<T> specialization instead, using the
// documented return-by-value pattern for non-default-constructible types.
namespace nlohmann {

template <>
struct adl_serializer<Manipulation::IDTrigger> {
    static void to_json(json& j, const Manipulation::IDTrigger& t)
    {
        j = json{{"id", t.id}};
    }
    static auto from_json(const json& j) -> Manipulation::IDTrigger
    {
        return Manipulation::IDTrigger(j.at("id").get<uint16_t>());
    }
};

template <>
struct adl_serializer<Manipulation::DLCTrigger> {
    static void to_json(json& j, const Manipulation::DLCTrigger& t)
    {
        j = json{{"dlc", t.dlc}};
    }
    static auto from_json(const json& j) -> Manipulation::DLCTrigger
    {
        return Manipulation::DLCTrigger(j.at("dlc").get<uint8_t>());
    }
};

template <>
struct adl_serializer<Manipulation::RandomTrigger> {
    static void to_json(json& j, const Manipulation::RandomTrigger& t)
    {
        j = json{{"probability", t.probability}};
    }
    static auto from_json(const json& j) -> Manipulation::RandomTrigger
    {
        return Manipulation::RandomTrigger(j.at("probability").get<float>());
    }
};

template <>
struct adl_serializer<Manipulation::SignalNameTrigger> {
    static void to_json(json& j, const Manipulation::SignalNameTrigger& t)
    {
        j = json{{"signal_name", t.signal_name}};
    }
    static auto from_json(const json& j) -> Manipulation::SignalNameTrigger
    {
        return Manipulation::SignalNameTrigger(j.at("signal_name").get<std::string>());
    }
};

template <>
struct adl_serializer<Manipulation::SignalThresholdTrigger> {
    static void to_json(json& j, const Manipulation::SignalThresholdTrigger& t)
    {
        j = json{
            {"signal_name", t.signal_name}, {"threshold", t.threshold}, {"isGreater", t.isGreater}};
    }
    static auto from_json(const json& j) -> Manipulation::SignalThresholdTrigger
    {
        return Manipulation::SignalThresholdTrigger(j.at("signal_name").get<std::string>(),
                                                    j.at("threshold").get<double>(),
                                                    j.at("isGreater").get<bool>());
    }
};

template <>
struct adl_serializer<Manipulation::BitFlipEffect> {
    static void to_json(json& j, const Manipulation::BitFlipEffect& t)
    {
        j = json{{"byteIndex", t.byteIndex}, {"bitIndex", t.bitIndex}};
    }
    static auto from_json(const json& j) -> Manipulation::BitFlipEffect
    {
        return Manipulation::BitFlipEffect(j.at("byteIndex").get<uint8_t>(),
                                           j.at("bitIndex").get<uint8_t>());
    }
};

template <>
struct adl_serializer<Manipulation::ValueSetEffect> {
    static void to_json(json& j, const Manipulation::ValueSetEffect& t)
    {
        j = json{{"signal_name", t.signal_name}, {"value", t.value}};
    }
    static auto from_json(const json& j) -> Manipulation::ValueSetEffect
    {
        return Manipulation::ValueSetEffect(j.at("signal_name").get<std::string>(),
                                            j.at("value").get<double>());
    }
};

template <>
struct adl_serializer<Manipulation::ClampEffect> {
    static void to_json(json& j, const Manipulation::ClampEffect& t)
    {
        j = json{
            {"signal_name", t.signal_name}, {"min_value", t.min_value}, {"max_value", t.max_value}};
    }
    static auto from_json(const json& j) -> Manipulation::ClampEffect
    {
        return Manipulation::ClampEffect(j.at("signal_name").get<std::string>(),
                                         j.at("min_value").get<double>(),
                                         j.at("max_value").get<double>());
    }
};

template <>
struct adl_serializer<Manipulation::NoiseEffect> {
    static void to_json(json& j, const Manipulation::NoiseEffect& t)
    {
        j = json{{"signal_name", t.signal_name}, {"amplitude", t.amplitude}};
    }
    static auto from_json(const json& j) -> Manipulation::NoiseEffect
    {
        return Manipulation::NoiseEffect(j.at("signal_name").get<std::string>(),
                                         j.at("amplitude").get<double>());
    }
};

template <>
struct adl_serializer<Manipulation::DelayedStrategy> {
    static void to_json(json& j, const Manipulation::DelayedStrategy& t)
    {
        j = json{{"delayUs", t.delayUs}};
    }
    static auto from_json(const json& j) -> Manipulation::DelayedStrategy
    {
        return Manipulation::DelayedStrategy(j.at("delayUs").get<uint32_t>());
    }
};

}  // namespace nlohmann

namespace Manipulation {

// The remaining leaf types have no invariants to validate, so they're default-constructible
// and can use plain ADL to_json/from_json instead of an adl_serializer specialization.

inline void to_json(nlohmann::json& j, const RandomBitFlipEffect& /*value*/)
{
    j = nlohmann::json::object();
}
inline void from_json(const nlohmann::json& /*j*/, RandomBitFlipEffect& /*value*/) {}

inline void to_json(nlohmann::json& j, const DropStrategy& /*value*/)
{
    j = nlohmann::json::object();
}
inline void from_json(const nlohmann::json& /*j*/, DropStrategy& /*value*/) {}

inline void to_json(nlohmann::json& j, const NoMutation& /*value*/)
{
    j = nlohmann::json::object();
}
inline void from_json(const nlohmann::json& /*j*/, NoMutation& /*value*/) {}

inline void to_json(nlohmann::json& j, const LatchMutation& /*value*/)
{
    j = nlohmann::json::object();
}
inline void from_json(const nlohmann::json& /*j*/, LatchMutation& /*value*/) {}

template <typename Effect>
void to_json(nlohmann::json& j, const EffectStrategy<Effect>& value)
{
    j = nlohmann::json{{"effects", value.effects}};
}

template <typename Effect>
void from_json(const nlohmann::json& j, EffectStrategy<Effect>& value)
{
    value.effects = j.at("effects").template get<std::vector<Effect>>();
}

inline void to_json(nlohmann::json& j, const DbcInsertStrategy& value)
{
    j = nlohmann::json{{"delayUs", value.delayUs}, {"message", value.message}};
}

inline void from_json(const nlohmann::json& j, DbcInsertStrategy& value)
{
    value.delayUs = j.at("delayUs").get<uint32_t>();
    value.message = j.at("message").get<std::optional<Core::DbcCanMessage>>();
}

namespace detail {

/**
 * @brief Stable JSON type tag per manipulation leaf/aggregate type, used to encode and decode
 * the std::variant aliases in manipulation.hpp as a tagged {"type", "value"} object.
 */
template <typename T>
auto typeTag() -> std::string_view;

#define MANIPULATION_TYPE_TAG(T)                 \
    template <>                                  \
    inline auto typeTag<T>() -> std::string_view \
    {                                            \
        return #T;                               \
    }

MANIPULATION_TYPE_TAG(IDTrigger)
MANIPULATION_TYPE_TAG(DLCTrigger)
MANIPULATION_TYPE_TAG(SignalNameTrigger)
MANIPULATION_TYPE_TAG(SignalThresholdTrigger)
MANIPULATION_TYPE_TAG(RandomTrigger)
MANIPULATION_TYPE_TAG(BitFlipEffect)
MANIPULATION_TYPE_TAG(RandomBitFlipEffect)
MANIPULATION_TYPE_TAG(ValueSetEffect)
MANIPULATION_TYPE_TAG(ClampEffect)
MANIPULATION_TYPE_TAG(NoiseEffect)
MANIPULATION_TYPE_TAG(RawEffectStrategy)
MANIPULATION_TYPE_TAG(DbcEffectStrategy)
MANIPULATION_TYPE_TAG(DelayedStrategy)
MANIPULATION_TYPE_TAG(DropStrategy)
MANIPULATION_TYPE_TAG(DbcInsertStrategy)
MANIPULATION_TYPE_TAG(NoMutation)
MANIPULATION_TYPE_TAG(LatchMutation)
MANIPULATION_TYPE_TAG(RawManipulation)
MANIPULATION_TYPE_TAG(DbcManipulation)

#undef MANIPULATION_TYPE_TAG

template <typename Variant>
void variantToJson(nlohmann::json& j, const Variant& v)
{
    std::visit(
        [&j](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            j = nlohmann::json{{"type", std::string(typeTag<T>())}, {"value", value}};
        },
        v);
}

/**
 * @brief Reconstructs a variant from its tagged JSON form.
 *
 * Built via std::optional rather than a default-constructed Variant, since several
 * alternatives (e.g. IDTrigger, SignalNameTrigger) have no default constructor, which would
 * otherwise make the variant itself non-default-constructible.
 */
template <typename Variant>
auto variantFromJson(const nlohmann::json& j) -> Variant
{
    const auto type = j.at("type").get<std::string>();
    const auto& value = j.at("value");

    std::optional<Variant> result;
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (void)(((type == typeTag<std::variant_alternative_t<Is, Variant>>())
                    ? (result.emplace(
                           value.template get<std::variant_alternative_t<Is, Variant>>()),
                       true)
                    : false) ||
               ...);
    }(std::make_index_sequence<std::variant_size_v<Variant>>{});

    if (!result)
    {
        throw std::runtime_error("Unknown manipulation type tag: " + type);
    }
    return std::move(*result);
}

}  // namespace detail

}  // namespace Manipulation

// The variant aliases mix default-constructible and non-default-constructible alternatives
// (e.g. RawTrigger's first alternative IDTrigger has no default constructor, which would
// otherwise make std::variant's own default constructor - and so get<T>()'s usual path -
// unavailable). So, like the leaf types above, each gets a full adl_serializer<T>
// specialization using the return-by-value from_json pattern, built on the visit/tag helpers
// in Manipulation::detail. This block must precede any get<T>() use of these variants below
// (e.g. inside RawManipulation/DbcManipulation's from_json), since those are plain inline
// functions whose bodies - and therefore their get<T>() instantiations - are compiled as soon
// as they're parsed, not deferred like a template's would be.
namespace nlohmann {

#define MANIPULATION_VARIANT_SERIALIZER(Variant)                                    \
    template <>                                                                     \
    struct adl_serializer<Manipulation::Variant> {                                  \
        static void to_json(json& j, const Manipulation::Variant& v)                \
        {                                                                           \
            Manipulation::detail::variantToJson(j, v);                              \
        }                                                                           \
        static auto from_json(const json& j) -> Manipulation::Variant               \
        {                                                                           \
            return Manipulation::detail::variantFromJson<Manipulation::Variant>(j); \
        }                                                                           \
    };

MANIPULATION_VARIANT_SERIALIZER(DbcTrigger)
MANIPULATION_VARIANT_SERIALIZER(RawTrigger)
MANIPULATION_VARIANT_SERIALIZER(DbcEffect)
MANIPULATION_VARIANT_SERIALIZER(RawEffect)
MANIPULATION_VARIANT_SERIALIZER(RawStrategy)
MANIPULATION_VARIANT_SERIALIZER(DbcStrategy)
MANIPULATION_VARIANT_SERIALIZER(Mutation)

#undef MANIPULATION_VARIANT_SERIALIZER

}  // namespace nlohmann

namespace Manipulation {

inline void to_json(nlohmann::json& j, const RawManipulation& value)
{
    j = nlohmann::json{
        {"trigger", value.trigger}, {"strategy", value.strategy}, {"mutation", value.mutation}};
}

inline void from_json(const nlohmann::json& j, RawManipulation& value)
{
    value.trigger = j.at("trigger").get<std::vector<RawTrigger>>();
    value.strategy = j.at("strategy").get<RawStrategy>();
    value.mutation = j.at("mutation").get<Mutation>();
}

inline void to_json(nlohmann::json& j, const DbcManipulation& value)
{
    j = nlohmann::json{
        {"trigger", value.trigger}, {"strategy", value.strategy}, {"mutation", value.mutation}};
}

inline void from_json(const nlohmann::json& j, DbcManipulation& value)
{
    value.trigger = j.at("trigger").get<std::vector<DbcTrigger>>();
    value.strategy = j.at("strategy").get<DbcStrategy>();
    value.mutation = j.at("mutation").get<Mutation>();
}

}  // namespace Manipulation

// ManipulationEntry wraps RawManipulation/DbcManipulation, so its serializer can only be
// defined once those two have their own to_json/from_json in scope (see comment above).
namespace nlohmann {

template <>
struct adl_serializer<Manipulation::ManipulationEntry> {
    static void to_json(json& j, const Manipulation::ManipulationEntry& v)
    {
        Manipulation::detail::variantToJson(j, v);
    }
    static auto from_json(const json& j) -> Manipulation::ManipulationEntry
    {
        return Manipulation::detail::variantFromJson<Manipulation::ManipulationEntry>(j);
    }
};

}  // namespace nlohmann

namespace Manipulation {

/**
 * @brief Saves/loads the manipulation list configured in a ManipulationModel to/from disk.
 */
class ManipulationSerializer
{
   public:
    static auto save(const std::filesystem::path& path,
                     const std::vector<ManipulationEntry>& entries) -> bool;
    static auto load(const std::filesystem::path& path) -> std::vector<ManipulationEntry>;
};

}  // namespace Manipulation
