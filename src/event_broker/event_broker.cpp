#include "event_broker.hpp";
namespace EventBroker {

auto EventBroker::_subscribe(std::type_index type,
                            std::function<void(const void*)> callback)
    -> Core::Connection
{
    auto &channel = getChannel(type);

    // Wrap std::function into a callable object
    struct Wrapper {
        std::function<void(const void*)> cb;

        void operator()(const void *& data) {
            cb(data);
        }
    };

    auto wrapper = std::make_shared<Wrapper>(Wrapper{std::move(callback)});

    entt::connection conn =
        channel.sink().connect<&Wrapper::operator()>(*wrapper);

    return Core::Connection(
        [c = std::move(conn), w = std::move(wrapper)]() mutable {
            c.release();
        }
    );
}

void EventBroker::_publish(std::type_index type, const void* data) {
    getChannel(type).trigger(data);
}
} // namespace EventBroker