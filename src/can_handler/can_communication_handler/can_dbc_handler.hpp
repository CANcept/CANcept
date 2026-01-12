//
// Created by flori on 28.12.2025.
//

#ifndef CANBUSMANAGER_CAN_DBC_HANDLER_HPP
#define CANBUSMANAGER_CAN_DBC_HANDLER_HPP
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "i_can_parser.hpp"

namespace CanHandler {
/**
 * @brief Handles the parsing of all CAN messages related with a DBC configuration.
 * It provides two main functionalities:
 * It listens to events of the event broker on sending dbc based messages. It then encodes the
 * messages based on the current DBC configuration and publishes the to the CAN device via the
 * CanCommunication handler. It furthermore translates incoming messages based on the current DBC
 * config into the physical values and publishes them to the event broker
 */
class CanDbcHandler final : public ICanParser
{
   public:
    explicit CanDbcHandler(Core::IEventBroker& eventBroker,
                           const std::function<bool(const CanMessage&)>& sendFunction)
        : ICanParser(eventBroker, sendFunction)
    {
        dbcSendEventConnection = eventBroker.subscribe<Core::SendCanMessageDbcEvent>(
            [this](const Core::SendCanMessageDbcEvent& event) -> void {
                handleSendMessage(event);
            });
        dbcConfigChangeConnection = eventBroker.subscribe<Core::DBCParsedEvent>(
            [this](const Core::DBCParsedEvent& event) -> void { handleNewDbc(event); });
    };
    ~CanDbcHandler() override = default;

   private:
    /**
     * @brief Parses a CAN message based on the current DBC config, publishes the parsed message to
     * the event broker
     * @param canMessage The message to be parsed
     */
    void parseReceivedMessage(const sockcanpp::CanMessage* canMessage) override;
    /**
     * @brief Encodes a dbc based decoded message into CAN form. It then publishes it to the CAN
     * device via the CanCommunicationHandler.
     * @param event The decoded message to be published
     */
    void handleSendMessage(const Core::SendCanMessageDbcEvent& event);
    /**
     * @brief Updates the currently stored DBC config.
     * @param event The new DBC config
     */
    void handleNewDbc(const Core::DBCParsedEvent& event);

    /**
     * @brief The connection containing the subscription to sending dbc based CAN message events
     */
    Core::Connection dbcSendEventConnection;
    /**
     * @brief The connection containing the subscription to new DBC configs
     */
    Core::Connection dbcConfigChangeConnection;
    /**
     * @brief The current DBC configuration
     */
    Core::DbcConfig currentConfig;
};
}  // namespace CanHandler

#endif  // CANBUSMANAGER_CAN_DBC_HANDLER_HPP
