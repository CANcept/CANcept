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

#include <concepts>
#include <list>
#include <string>
#include <type_traits>

#include "core/event/settings_event.hpp"
#include "core/interface/i_event_broker.hpp"

namespace Core {

/** @brief Constrains TEvent to types derived from SelectProviderOptionEvent. */
template <typename T>
concept SelectProviderEvent = std::derived_from<T, SelectProviderOptionEvent>;

/** @brief Primary template for type-specific setting metadata. */
template <SettingType TType, typename TEvent>
struct TypeTraits;

/** @brief Metadata for select-type settings carrying a provider event. */
template <SelectProviderEvent TEvent>
struct TypeTraits<SettingType::Select, TEvent> {
    using ProviderEvent = TEvent;
    std::string placeholder;
    explicit TypeTraits(const std::string& placeholder) : placeholder(placeholder) {}
};

/** @brief Type-erased base for all settings stored in the registry. */
struct ISetting {
    virtual ~ISetting() = default;

    /** @brief Returns the unique key identifying this setting. */
    [[nodiscard]] virtual auto getKey() const -> const SettingKey& = 0;

    /** @brief Returns the setting category. */
    [[nodiscard]] virtual auto getType() const -> SettingType = 0;

    /** @brief Returns the icon resource path. */
    [[nodiscard]] virtual auto getIcon() const -> const std::string& = 0;

    /** @brief Returns a placeholder or label hint for the UI. */
    [[nodiscard]] virtual auto getPlaceholder() const -> const std::string&
    {
        static const std::string empty;
        return empty;
    }

    /** @brief Fetches dynamic options for select-type settings via the event broker. */
    [[nodiscard]] virtual auto fetchOptions(IEventBroker& broker) const -> std::list<SelectOption>
    {
        return {};
    }

    /** @brief Publishes a change event when the setting value is updated. */
    virtual void publishChanged(IEventBroker& broker, const std::string& value) const {}
};

/** @brief Complete definition of a setting instance. */
template <SettingType TType, typename TEvent, typename TChangedEvent = void>
struct SettingDefinition final : public ISetting {
    SettingKey key;
    std::string icon;
    TypeTraits<TType, TEvent> traits;

    [[nodiscard]] auto getKey() const -> const SettingKey& override
    {
        return key;
    }
    [[nodiscard]] auto getType() const -> SettingType override
    {
        return TType;
    }
    [[nodiscard]] auto getIcon() const -> const std::string& override
    {
        return icon;
    }
};

/** @brief Select-type specialization that fetches options via a typed provider event. */
template <SelectProviderEvent TEvent, typename TChangedEvent>
struct SettingDefinition<SettingType::Select, TEvent, TChangedEvent> final : public ISetting {
    SettingKey key;
    std::string icon;
    TypeTraits<SettingType::Select, TEvent> traits;

    [[nodiscard]] auto getKey() const -> const SettingKey& override
    {
        return key;
    }
    [[nodiscard]] auto getType() const -> SettingType override
    {
        return SettingType::Select;
    }
    [[nodiscard]] auto getIcon() const -> const std::string& override
    {
        return icon;
    }

    [[nodiscard]] auto getPlaceholder() const -> const std::string& override
    {
        return traits.placeholder;
    }

    [[nodiscard]] auto fetchOptions(IEventBroker& broker) const -> std::list<SelectOption> override
    {
        std::list<SelectOption> options;
        broker.publish(TEvent(&options));
        return options;
    }

    void publishChanged(IEventBroker& broker, const std::string& value) const override
    {
        if constexpr (!std::is_void_v<TChangedEvent>)
        {
            broker.publish(TChangedEvent(value));
        }
    }

    SettingDefinition(const SettingKey& key, const std::string& icon,
                      const TypeTraits<SettingType::Select, TEvent>& traits)
        : key(key), icon(icon), traits(traits)
    {
    }
};

}  // namespace Core
