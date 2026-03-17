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
#include "core/event/lifecycle_event.hpp"
#include "i_event_broker.hpp"
#include "i_settings_registry.hpp"

namespace Core {

/**
 * @brief Base class providing unified lifecycle behavior for application components.
 *
 * Components that participate in the application lifecycle should derive from this class.
 * It automatically registers lifecycle handlers with the event broker.
 */
class ILifecycle
{
   public:
    /**
     * @brief Constructs the lifecycle component and stores event subscriptions.
     *
     * @param broker Reference to the application's event broker.
     * @note The subscriptions are tied to the lifetime of this object instance.
     */
    explicit ILifecycle(IEventBroker& broker) : m_eventBroker(broker)
    {
        m_startConn = m_eventBroker.subscribe<AppStartedEvent>(
            [this](const AppStartedEvent&) -> void { onStart(); });

        m_stopConn = m_eventBroker.subscribe<AppStoppedEvent>(
            [this](const AppStoppedEvent&) -> void { onStop(); });
    }

    /**
     * @brief Virtual destructor.
     * * The m_startConn and m_stopConn members will automatically trigger
     * unsubscription during destruction.
     */
    virtual ~ILifecycle() = default;

    /**
     * @brief ILifecycle components should not be copied.
     */
    ILifecycle(const ILifecycle&) = delete;
    /**
     * @brief ILifecycle components should typically not be copied.
     */
    auto operator=(const ILifecycle&) -> ILifecycle& = delete;
    /**
     * @brief This allows the extending component to register settings.
     */
    virtual void registerSettings(ISettingsRegistry& registry) {}

   protected:
    /**
     * @brief Called automatically when the application publishes AppStartedEvent.
     */
    virtual void onStart() = 0;

    /**
     * @brief Called automatically when the application publishes AppStoppedEvent.
     */
    virtual void onStop() = 0;

    /**
     * @brief Reference to the shared event broker for publishing events.
     */
    IEventBroker& m_eventBroker;

   private:
    /**
     * @brief Handle for the AppStartedEvent subscription.
     */
    Connection m_startConn;
    /**
     * @brief Handle for the AppStoppedEvent subscription.
     */
    Connection m_stopConn;
};

}  // namespace Core