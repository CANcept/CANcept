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

#include <QString>
#include <QStringList>
#include <Qt>
#include <QtGlobal>

/**
 * @namespace Constants
 * @brief Module-level constants for CAN message sending configuration.
 */
namespace Monitoring::Constants {

/** @brief Icon used for the monitoring module tab */
inline const QString TAB_ICON_PATH = ":/assets/icon/monitoring/monitoring_tab.svg";

/** @brief Icon used for the signal graph cards */
inline const QString SIGNAL_GRAPH_ICON_PATH = ":/assets/icon/monitoring/signal_graph.svg";

/** @brief Icon used for the can configuration card */
inline const QString CAN_CONFIGURATION_ICON_PATH = ":/assets/icon/monitoring/bus_config.svg";

/** @brief Icon used for the connected button in the can config card */
inline const QString BUS_CONNECT_BUTTON_ICON_PATH = ":/assets/icon/monitoring/bus_connected.svg";

/** @brief Icon used for the disconnected button in the can config card */
inline const QString BUS_DISCONNECT_BUTTON_ICON_PATH =
    ":/assets/icon/monitoring/bus_disconnected.svg";

/** @brief Icon used for the message card in not expanded state */
inline const QString ARROW_RIGHT_BUTTON_ICON_PATH = ":/assets/icon/monitoring/arrow_right.svg";

/** @brief Icon used for the message card in expanded state */
inline const QString ARROW_DOWN_BUTTON_ICON_PATH = ":/assets/icon/monitoring/arrow_down.svg";

/** @brief Path to the settings Logo. */
inline const QString SETTINGS_ICON_PATH = ":/assets/icon/settings.svg";

/** @brief Identifier used by the module to communicate with the system */
inline const QString MODULE_IDENTIFIER = "MonitoringComponent";

/** @brief Title shown in the tab bar */
inline const QString TAB_TITLE = "Monitoring";

/** @brief Card title of the configuration Card. */
inline const QString CAN_CONFIGURATION_TITLE = "CAN-Bus Connection";

/** @brief Label for the interface selection card */
inline const QString INTERFACE_LABEL = "Interface";

/** @brief Placeholder text for the interface combo box */
inline const QString INTERFACE_PLACEHOLDER = "Select interface...";

/** @brief Label for the baud rate selection card */
inline const QString CAN_STATUS_LABEL = "Status";

/** @brief Label for the baud rate selection card */
inline const QString FRAME_RATE_LABEL = "Frame Rate: ";

/** @brief Placeholder text for the baud rate combo box */
inline const QString FRAME_RATE_PLACEHOLDER = "--";

/** @brief Label for the baud rate selection card */
inline const QString MESSAGE_COUNT_LABEL = "Message types: ";

/** @brief Label for the baud rate selection card */
inline const QString CAN_CONFIG_CONNECTED_LABEL = "CONNECTED";

/** @brief Label for the baud rate selection card */
inline const QString BUS_CONNECT_BUTTON_LABEL = "Connect";

/** @brief Placeholder text for the baud rate combo box */
inline const QString CAN_CONFIG_DISCONNECTED_LABEL = "DISCONNECTED";

/** @brief Label for signal graph card */
inline const QString SIGNAL_GRAPH_TITLE = "Signal Value";

/** @brief Label for messages list in view */
inline const QString MESSAGES_LABEL = "Message";

/** @brief Text for when a dbc file isn't loaded. */
inline const QString DBC_FILE_NOT_LOADED_MESSAGE =
    "Connection not complete: Load DBC and select interface in ";

const int REFRESH_INTERVAL_MS = 1000;

const int HOLDING_SECONDS_IN_MODEL = 60;

}  // namespace Monitoring::Constants

// namespace Sending
