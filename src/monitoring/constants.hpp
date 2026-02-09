#pragma once

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

/** @brief Placeholder text for the baud rate combo box */
inline const QString MESSAGE_COUNT_PLACEHOLDER = "--";

/** @brief Label for the baud rate selection card */
inline const QString CAN_CONFIG_CONNECTED_LABEL = "CONNECTED";

/** @brief Label for the baud rate selection card */
inline const QString BUS_CONNECT_BUTTON_LABEL = "Connect";

/** @brief Placeholder text for the baud rate combo box */
inline const QString CAN_CONFIG_DISCONNECTED_LABEL = "DISCONNECTED";

/** @brief Label for signal graph card */
inline const QString SIGNAL_GRAPH_TITLE = "Signal Value";

/** @brief Default list of CAN interfaces to populate in device selectors */
inline const QStringList DEFAULT_CAN_DEVICES = {"vcan0", "can0", "can1"};

/** @brief Default painting color for signal graphs */
inline const Qt::GlobalColor SIGNAL_GRAPH_LINE_COLOR = Qt::blue;

/** @brief Default width for line of signal graphs */
inline const int SIGNAL_GRAPH_LINE_WIDTH = 2;

/** @brief Label for messages list in view */
inline const QString MESSAGES_LABEL = "Message";

const int REFRESH_INTERVAL_MS = 1000;

const int HOLDING_SECONDS_IN_MODEL = 60;

}  // namespace Monitoring::Constants

// namespace Sending
