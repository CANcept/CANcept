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

#pragma once
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
        : ICanParser(eventBroker, sendFunction), dbcMessages{}
    {
        dbcSendEventConnection = eventBroker.subscribe<Core::EncodeCanMessageDbcEvent>(
            [this](const Core::EncodeCanMessageDbcEvent& event) -> void {
                handleEncodeMessage(event);
            });
        dbcConfigChangeConnection = eventBroker.subscribe<Core::DBCParsedEvent>(
            [this](const Core::DBCParsedEvent& event) -> void { handleNewDbc(event); });

        dbcMessages.fill(nullptr);
    };
    ~CanDbcHandler() override;

    /**
     * @brief Parses a CAN message based on the current DBC config, publishes the parsed message to
     * the event broker
     * @param canMessage The message to be parsed
     */
    void parseReceivedMessage(const sockcanpp::CanMessage* canMessage) override;

   private:
    /**
     * @brief Encodes a decoded DBC message into a raw CAN frame and populates event.encodedMessage.
     * @param event The encode request containing the decoded message and the output reference.
     */
    void handleEncodeMessage(const Core::EncodeCanMessageDbcEvent& event);
    /**
     * @brief Updates the currently stored DBC config.
     * @param event The new DBC config
     */
    void handleNewDbc(const Core::DBCParsedEvent& event);

    /**
     * @brief Parses a received signal based on a signal description
     * @param signal The signal description for the signal
     * @param dataLittleEndian The received data in little endian form
     * @param dataBigEndian The received data in big endian form
     * @return The parsed signal value
     */
    static auto parseReceivedSignal(const Core::DbcSignalDescription& signal,
                                    const u_int64_t& dataLittleEndian,
                                    const u_int64_t& dataBigEndian) -> double;

    /**
     * @brief Parses a signal to send based on a signal description
     * @param signal The signal description for the signal
     * @param dataLittleEndian The accumulated, parsed little endian data of the message in little
     * endian form
     * @param dataBigEndian The accumulated, parsed big endian data of the message in big endian
     * form
     * @param value The value of the signal to parse
     */
    static void parseSendSignal(const Core::DbcSignalDescription& signal,
                                u_int64_t& dataLittleEndian, u_int64_t& dataBigEndian,
                                const double& value);
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
    std::array<Core::DbcMessageDescription*, 2048> dbcMessages;
    /**
     * @brief Mutex guarding the dbc configuration
     */
    std::mutex dbcMutex;
};
}  // namespace CanHandler
