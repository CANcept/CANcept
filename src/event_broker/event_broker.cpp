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

#include "event_broker.hpp"
namespace EventBroker {

auto EventBroker::_subscribe(const std::type_index type, std::function<void(const void*)> callback)
    -> Core::Connection
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
