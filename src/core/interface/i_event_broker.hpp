#pragma once
#include <functional>
#include <typeindex>
#include <utility>

namespace Core {

/**
 * @brief A RAII-style handle for managing event subscriptions.
 */
class [[nodiscard]] Connection
{
   public:
    Connection() = default;

    /**
     * @brief Internal constructor used by the Broker to wrap unsubscription logic.
     * @param disconnect A lambda that, when called, removes the listener from the Broker.
     */
    explicit Connection(std::function<void()> disconnect) : m_disconnect(std::move(disconnect)) {}

    /**
     * @brief The Destructor ensures safety.
     * When this handle goes out of scope, it automatically unsubscribes the listener.
     */
    ~Connection()
    {
        release();
    }

    /** @brief Copying is disabled to prevent multiple objects managing the same lifetime. */
    Connection(const Connection&) = delete;
    auto operator=(const Connection&) -> Connection& = delete;

    /** @brief Moving transfers the subscription ownership to a new handle. */
    Connection(Connection&& other) noexcept : m_disconnect(std::move(other.m_disconnect))
    {
        other.m_disconnect = nullptr;
    }

    /** @brief Moving updates the ownership, releasing any existing subscription first. */
    auto operator=(Connection&& other) noexcept -> Connection&
    {
        if (this != &other)
        {
            release();
            m_disconnect = std::move(other.m_disconnect);
            other.m_disconnect = nullptr;
        }
        return *this;
    }

    /**
     * @brief Manually triggers unsubscription and clears the internal state.
     */
    void release()
    {
        if (m_disconnect)
        {
            m_disconnect();
            m_disconnect = nullptr;
        }
    }

    /** @brief Checks if the handle currently manages an active subscription. */
    explicit operator bool() const
    {
        return m_disconnect != nullptr;
    }

   private:
    std::function<void()> m_disconnect;
};

/**
 * @brief Interface for a central Event Broker (Event Bus).
 * * @details
 * This interface provides a type-safe way for decoupled modules to communicate.
 * It is designed to be a wrapper around systems like 'entt::dispatcher'.
 */
class IEventBroker
{
   public:
    virtual ~IEventBroker() = default;

    /**
     * @brief Dispatches an event to all registered listeners immediately.
     * * @tparam Event The event structure type.
     * @param event The event instance containing the data to be sent.
     */
    template <typename Event>
    void publish(const Event& event)
    {
        _publish(typeid(Event), &event);
    }

    /**
     * @brief Registers a callback function for a specific event type.
     * * @tparam Event The event structure type to listen for.
     * @param callback The function to execute when the event occurs.
     * @return A Connection handle. You MUST store this handle; if it is destroyed,
     * the subscription ends immediately.
     */
    template <typename Event>
    auto subscribe(std::function<void(const Event&)> callback) -> Connection
    {
        return _subscribe(typeid(Event), [cb = std::move(callback)](const void* data) -> auto {
            cb(*static_cast<const Event*>(data));
        });
    }

   protected:
    /**
     * @brief Implementation-specific logic for triggering events.
     */
    virtual void _publish(std::type_index type, const void* data) = 0;

    /**
     * @brief Implementation-specific logic for storing listeners.
     */
    virtual auto _subscribe(std::type_index type, std::function<void(const void*)> callback)
        -> Connection = 0;
};

}  // namespace Core
