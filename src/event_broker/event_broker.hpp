//
// Created by Lino on 17.12.2025.
//

#ifndef CANBUSSIMULATOR_EVENTBROKER_HPP
#define CANBUSSIMULATOR_EVENTBROKER_HPP

#include <entt/entt.hpp>

#include "core/interface/i_event_broker.hpp"
namespace EventBroker {
/**
 * @brief A implementation of the @code IEventBroker. Implements the virtual methods of the
 * IEventBroker to subscribe to and publish events.
 * @details It is based on Entt dispatchers: For every event type a dispatcher is registered
 * It provides all logic to subscribe/publish events for that event type.
 */
class EventBroker final : public Core::IEventBroker
{
   protected:
    /**
     * @brief The method, that is called if an event should be published by the event broker
     * @param type The type ID of the event
     * @param data A pointer to the event
     */
    void _publish(std::type_index type, const void* data) override;
    /**
     * @brief The method, that is called if you want to subscribe to an event
     * @param type The type ID of the event
     * @param callback A callback function, that is called on that event
     * @return A connection, that is responsible for keeping the subscription alive. If destroyed
     * the subscription ends
     */
    auto _subscribe(std::type_index type,
                    std::function<void(const void*)> callback) -> Core::Connection override;

   private:
    /**
     * @brief A chanel is associated with one event type id and contains the dispatcher for that
     * event
     */
    struct Channel {
        entt::dispatcher dispatcher;

        /**
         * @brief Publishes an event to the dispatcher
         * @param event The event to be published
         */
        void trigger(const void* event)
        {
            dispatcher.trigger(event);
        }

        /**
         * @brief Returns the sink of the dispatcher for subscriptions
         * @return
         */
        auto sink()
        {
            return dispatcher.sink<const void*>();
        }
    };

    /**
     * @brief Gets the channel for a specific event type
     * @param type The type id of the event
     * @return The associated channel
     */
    auto getChannel(const std::type_index type) -> Channel&
    {
        auto& ptr = channels[type];
        if (!ptr)
        {
            ptr = std::make_unique<Channel>();
        }
        return *ptr;
    }

    /**
     * @brief A map containing the associations between channels and event type ids
     */
    std::unordered_map<std::type_index, std::unique_ptr<Channel>> channels;
};
}  // namespace EventBroker

#endif  // CANBUSSIMULATOR_EVENTBROKER_HPP