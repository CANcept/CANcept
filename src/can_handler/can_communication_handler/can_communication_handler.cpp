#include "can_communication_handler.hpp"
namespace CanHandler {
void CanCommunicationHandler::checkCanDeviceForMessages() const
{
    for (const std::list<CanMessage> messages = deviceHandler->checkForCanMessage();
         CanMessage message : messages)
    {
        for (const std::unique_ptr<ICanParser> &parser : can_handlers)
        {
            parser->parseReceivedMessage(&message);
        }
    }
}
void CanCommunicationHandler::onStart()
{
    // Create a thread that checks on the messages every 100 ms
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
    for (std::unique_ptr<ICanParser> &parser : can_handlers)
    {
        parser.reset();
    }
    deviceHandler.reset();
}

void CanCommunicationHandler::registerSettings(Core::ISettingsRegistry &registry)
{
    deviceHandler->registerSettings(registry);
}

}  // namespace CanHandler