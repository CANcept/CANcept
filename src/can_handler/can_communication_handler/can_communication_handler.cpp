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

#include "can_communication_handler.hpp"

#include "core/macro/console_logging.hpp"

namespace CanHandler {
void CanCommunicationHandler::checkCanDeviceForMessages() const
{
    for (const std::list<CanMessage> messages = deviceHandler->checkForCanMessage();
         CanMessage message : messages)
    {
        for (const std::unique_ptr<ICanParser>& parser : can_handlers)
        {
            parser->parseReceivedMessage(&message);
        }
    }
}
void CanCommunicationHandler::onStart()
{
    LOG_INF("CanCommunicationHandler", "Starting CAN message polling thread");

    // Create a thread that checks on the messages every 5ms
    _execute = true;
    message_check_thread = std::thread([this]() {
        long long lastExecution = 0;
        while (_execute)
        {
            const auto millisecondsBeforeParse =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
            if (millisecondsBeforeParse > lastExecution + MILLISECONDS_BETWEEN_PARSE_ATTEMPTS)
            {
                lastExecution = millisecondsBeforeParse;

                checkCanDeviceForMessages();
            }
            const auto millisecondsAfterParse =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
            if (millisecondsAfterParse < lastExecution + MILLISECONDS_BETWEEN_PARSE_ATTEMPTS)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(
                    lastExecution + MILLISECONDS_BETWEEN_PARSE_ATTEMPTS - millisecondsAfterParse));
            }
        }
    });
}

void CanCommunicationHandler::onStop()
{
    _execute = false;
    message_check_thread.join();
}

void CanCommunicationHandler::registerSettings(Core::ISettingsRegistry& registry)
{
    deviceHandler->registerSettings(registry);
}

}  // namespace CanHandler