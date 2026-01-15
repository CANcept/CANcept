//
// Created by Florian on 14.01.2026.
//
#include "can_communication_handler.hpp"
namespace CanHandler {
void CanCommunicationHandler::checkCanDeviceForMessages() const
{
    for (const std::list<CanMessage> messages = deviceHandler.checkForCanMessage();
         CanMessage message : messages)
    {
        for (ICanParser parser : can_handlers)
        {
            parser.parseReceivedMessage(&message);
        }
    }
}
void CanCommunicationHandler::onStart()
{
    // Create thread, that checks on the messages every 100 ms
    _execute = true;
    message_check_thread = std::thread([this]() {
        long long lastExecution = 0;
        while (_execute)
        {
            const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                          std::chrono::system_clock::now().time_since_epoch())
                                          .count();
            if (milliseconds > lastExecution + 100)
            {
                lastExecution = milliseconds;

                checkCanDeviceForMessages();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}

void CanCommunicationHandler::onStop()
{
    _execute = false;
    message_check_thread.join();
    for (const ICanParser parser : can_handlers)
    {
        delete &parser;
    }
    delete &deviceHandler;
}
}  // namespace CanHandler