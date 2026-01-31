#include "event_broker.hpp"
namespace EventBroker {

auto EventBroker::_subscribe(const std::type_index type,
                             std::function<void(const void*)> callback) -> Core::Connection
{
    auto& channel = getChannel(type);

    // Wrap std::function into a callable object
    struct Wrapper {
        std::function<void(const void*)> callback;

        void operator()(const void*& data) const
        {
            callback(data);
        }
    };

    auto wrapper = std::make_shared<Wrapper>(Wrapper{std::move(callback)});

    entt::connection connection = channel.sink().connect<&Wrapper::operator()>(*wrapper);

    return Core::Connection([connection = std::move(connection),
                             wrapper = std::move(wrapper)]() mutable { connection.release(); });
}

void EventBroker::_publish(std::type_index type, const void* data)
{
    getChannel(type).trigger(data);
}
}  // namespace EventBroker