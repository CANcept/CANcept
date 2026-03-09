#pragma once

#include <gmock/gmock.h>

#include <functional>
#include <map>
#include <mutex>
#include <typeindex>
#include <vector>

#include "core/interface/i_event_broker.hpp"

namespace TestHelpers {

/**
 * @class MockEventBroker
 * @brief Mock implementation of IEventBroker for unit testing.
 *
 * @code
 * MockEventBroker broker;
 * EXPECT_CALL(broker, PublishEvent<MyEvent>(_)).Times(1);
 *
 * // Test code that publishes MyEvent
 * broker.publish(MyEvent{});
 *
 * // Manually trigger a subscription to test component response
 * broker.triggerEvent(MyEvent{42});
 * @endcode
 */
class MockEventBroker final : public Core::IEventBroker
{
   public:
    MockEventBroker() = default;
    ~MockEventBroker() override = default;

    // Delete copy constructor and assignment
    MockEventBroker(const MockEventBroker&) = delete;
    auto operator=(const MockEventBroker&) -> MockEventBroker& = delete;

    // Allow move
    MockEventBroker(MockEventBroker&&) noexcept = delete;
    auto operator=(MockEventBroker&&) noexcept -> MockEventBroker& = delete;

    /**
     * @brief Mock method for verifying publish calls.
     *
     * @code
     * EXPECT_CALL(broker, PublishEvent<MyEvent>(_)).Times(1);
     * @endcode
     */
    template <typename Event>
    void PublishEvent(const Event& event)
    {
        _publishEvent(typeid(Event), &event);
    }

    /**
     * @brief Mock method for verifying subscribe calls.
     *
     * @code
     * EXPECT_CALL(broker, SubscribeEvent<MyEvent>(_)).Times(1);
     * @endcode
     */
    template <typename Event>
    void SubscribeEvent(std::function<void(const Event&)> callback)
    {
        _subscribeEvent(typeid(Event));
    }

    /**
     * @brief Manually trigger an event to test subscribers.
     * This simulates receiving an event from the broker.
     *
     * @tparam Event The event type to trigger
     * @param event The event data
     */
    template <typename Event>
    void triggerEvent(const Event& event)
    {
        std::vector<std::function<void(const void*)>> snapshot;
        {
            std::lock_guard lock(m_mutex);
            if (const auto it = m_callbacks.find(typeid(Event)); it != m_callbacks.end())
            {
                snapshot = it->second;
            }
        }
        for (const auto& callback : snapshot)
        {
            if (callback)
            {
                callback(&event);
            }
        }
    }

    /**
     * @brief Check if there are any active subscriptions for an event type.
     */
    template <typename Event>
    auto hasSubscription() const -> bool
    {
        std::lock_guard lock(m_mutex);
        const auto it = m_callbacks.find(typeid(Event));
        return it != m_callbacks.end() && !it->second.empty();
    }

    /**
     * @brief Get the number of active subscriptions for an event type.
     */
    template <typename Event>
    auto subscriptionCount() const -> size_t
    {
        std::lock_guard lock(m_mutex);
        const auto it = m_callbacks.find(typeid(Event));
        return it != m_callbacks.end() ? it->second.size() : 0;
    }

    /**
     * @brief Clear all subscriptions (useful for test cleanup).
     */
    void clearSubscriptions()
    {
        std::lock_guard lock(m_mutex);
        m_callbacks.clear();
    }

    // GMock methods for verification
    MOCK_METHOD(void, _publishEvent, (std::type_index type, const void* data));
    MOCK_METHOD(void, _subscribeEvent, (std::type_index type));

   protected:
    /**
     * @brief Implementation of IEventBroker::_publish.
     * Thread-safe: callbacks are copied under the lock then invoked outside it,
     * so subscribers can safely publish or subscribe inside their callbacks.
     */
    void _publish(const std::type_index type, const void* data) override
    {
        std::vector<std::function<void(const void*)>> snapshot;
        {
            std::lock_guard lock(m_mutex);
            if (const auto it = m_callbacks.find(type); it != m_callbacks.end())
            {
                snapshot = it->second;
            }
        }
        for (const auto& function : snapshot)
        {
            if (function != nullptr)
            {
                function(data);
            }
        }
        _publishEvent(type, data);
    }

    /**
     * @brief Implementation of IEventBroker::_subscribe. Thread-safe.
     */
    auto _subscribe(std::type_index type,
                    std::function<void(const void*)> callback) -> Core::Connection override
    {
        _subscribeEvent(type);
        std::lock_guard lock(m_mutex);
        m_callbacks[type].push_back(std::move(callback));

        const size_t index = m_callbacks[type].size() - 1;
        return Core::Connection([this, type, index]() {
            std::lock_guard lock(m_mutex);
            if (const auto it = m_callbacks.find(type);
                it != m_callbacks.end() && index < it->second.size())
            {
                it->second[index] = nullptr;
            }
        });
    }

   private:
    mutable std::mutex m_mutex;
    std::map<std::type_index, std::vector<std::function<void(const void*)>>> m_callbacks;
};

}  // namespace TestHelpers
