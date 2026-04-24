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
#include <string>
#include <utility>

#include "event.hpp"
#include "settings_event.hpp"
namespace Core {
/**
 * @brief Event, that gets published if the name of the current CAN device changes.
 */
struct CanDriverChangeEvent final : public Event {
    /**
     * @brief The new name of the CAN driversNames.
     */
    std::string driverName;

    explicit CanDriverChangeEvent(std::string driverName) : driverName(std::move(driverName)) {};
};
/**
 * @brief Event, that gets called to get all currently available can drivers
 */
struct GetAvailableCanDriversEvent final : public SelectProviderOptionEvent {
    explicit GetAvailableCanDriversEvent(std::list<SelectOption>* options)
        : SelectProviderOptionEvent(options) {};
};
/**
 * @brief Event to check if the CAN device is ready for sending messages.
 */
struct CheckCanDeviceReadyEvent final : public Event {
    /**
     * @brief Reference to boolean that will be set to true if device is ready, false otherwise.
     */
    bool& isReady;

    explicit CheckCanDeviceReadyEvent(bool& ready) : isReady(ready) {}
};
}  // namespace Core
