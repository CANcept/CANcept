#include "event_broker.hpp"

namespace EventBroker {

// Stub _publish method
void EventBroker::_publish(const std::type_index type, const void* data)
{
    // do nothing for now
    (void)type;
    (void)data;
}

// Stub _subscribe method
auto EventBroker::_subscribe(const std::type_index type,
                             const std::function<void(const void*)> callback) -> Core::Connection
{
    // do nothing for now
    (void)type;
    (void)callback;
    return Core::Connection{};  // return default empty connection
}

}  // namespace EventBroker
