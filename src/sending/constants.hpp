#pragma once

#include <QRegularExpression>
#include <QStringLiteral>
#include <cstdint>
#include <string>
#include <vector>

namespace Sending {

/**
 * @namespace Constants
 * @brief Module-level constants for CAN message sending configuration.
 *
 * This namespace contains all magic numbers, strings, and configuration values
 * used throughout the Sending module. Constants are organized by category for
 * maintainability and follow the principle of "define once, use everywhere."
 */
namespace Constants {

// ICON PATHS

/** @brief Icon used for the main Sending tab */
inline const QString SENDING_ICON_PATH = ":/assets/icon/sending/sending.svg";

/** @brief Icon used for the send button */
inline const QString SEND_BUTTON_ICON_PATH = ":/assets/icon/sending/send.svg";

/** @brief Icon used for the raw sending sub-tab */
inline const QString RAW_SENDING_ICON_PATH = ":/assets/icon/sending/raw_based_send_icon.svg";

/** @brief Icon used for the DBC-based sending sub-tab */
inline const QString DBC_SENDING_ICON_PATH = ":/assets/icon/sending/dbc_based_send_icon.svg";

/** @brief Icon used for CAN-Bus configuration cards */
inline const QString CONFIGURATION_ICON_PATH = ":/assets/icon/sending/configuration.svg";

/** @brief Icon used for CAN frame input cards */
inline const QString CAN_FRAME_ICON_PATH = ":/assets/icon/sending/can_frame.svg";

/** @brief Icon used for CAN frame input cards */
inline const QString MESSAGES_ICON_PATH = ":/assets/icon/sending/messages.svg";

// MODULE IDENTIFICATION

/** @brief Unique identifier used by the module for event broker communication */
inline const std::string MODULE_IDENTIFIER = "SendingComponent";

/** @brief Title displayed in the main tab bar */
inline const QString TAB_TITLE = "Sending";

// UI TEXT & LABELS

/** @brief Title for the CAN-Bus configuration card */
inline const QString CAN_CONFIGURATION_TITLE = "CAN-Bus Configuration";

/** @brief Label for CAN frame input card */
inline const QString CAN_FRAME_TITLE = "CAN Frame";

/** @brief Description for CAN frame input card */
inline const QString CAN_FRAME_DESCRIPTION = "Enter CAN ID and message data as hexadecimal values";

/** @brief Label for CAN ID input field */
inline const QString CAN_ID_LABEL = "CAN ID (Hexadecimal)";

/** @brief Label for message data input field */
inline const QString MESSAGE_DATA_LABEL = "Message Data (max 8 bytes)";

/** @brief Label for interface selection */
inline const QString INTERFACE_LABEL = "Interface";

/** @brief Label for baud rate selection */
inline const QString BAUD_RATE_LABEL = "Baud Rate";

/** @brief Label for messages list in DBC-based view */
inline const QString MESSAGES_LABEL = "Messages";

/** @brief Text for raw sending mode button */
inline const QString RAW_MODE_BUTTON_TEXT = "Raw";

/** @brief Text for DBC-based sending mode button */
inline const QString DBC_MODE_BUTTON_TEXT = "DBC-Based";

/** @brief Text displayed on the send message button */
inline const QString SEND_BUTTON_TEXT = "Send Message";

// PLACEHOLDER TEXT

/** @brief Placeholder for interface selection combo box */
inline const QString INTERFACE_PLACEHOLDER = "Select interface...";

/** @brief Placeholder for baud rate selection combo box */
inline const QString BAUD_RATE_PLACEHOLDER = "Select baud rate...";

/** @brief Placeholder for CAN ID input field */
inline const QString CAN_ID_PLACEHOLDER = "1A2";

/** @brief Placeholder for message data input field */
inline const QString MESSAGE_DATA_PLACEHOLDER = "01 02 03 04 05 06 07 08";

// CAN PROTOCOL CONSTANTS

/** @brief Maximum number of data bytes in a standard CAN frame */
inline constexpr uint8_t MAX_CAN_DLC = 8;

/** @brief Minimum CAN ID value */
inline constexpr uint16_t MIN_CAN_ID = 0x000;

/** @brief Maximum CAN ID (11-bit standard identifier) */
inline constexpr uint16_t MAX_CAN_ID = 0x7FF;

/** @brief Maximum length for CAN ID hex input (e.g., "7FF" = 3 chars) */
inline constexpr int MAX_CAN_ID_HEX_LENGTH = 3;

// NETWORK CONFIGURATION

/** @brief Default list of CAN interfaces to populate in device selectors */
inline const std::vector<std::string> DEFAULT_CAN_INTERFACES = {"vcan0", "can0", "can1"};

/** @brief Standard CAN bit rates supported by most hardware (in bps) */
inline const std::vector<uint32_t> STANDARD_BIT_RATES = {125000, 250000, 500000, 1000000};

/** @brief Default cyclic transmission interval in milliseconds */
inline constexpr int DEFAULT_CYCLE_INTERVAL_MS = 100;

/** @brief Minimum allowed cyclic interval to prevent system overload */
inline constexpr int MIN_CYCLE_INTERVAL_MS = 10;

/** @brief Maximum allowed cyclic interval */
inline constexpr int MAX_CYCLE_INTERVAL_MS = 10000;

// HEX INPUT & VALIDATION

/** @brief Prefix displayed before hex CAN ID values */
inline const QString HEX_PREFIX = "0x";

/** @brief Width in pixels for rendering the "0x" prefix decoration */
inline constexpr int HEX_PREFIX_DISPLAY_WIDTH = 20;

/** @brief Number of hexadecimal characters per byte */
inline constexpr int HEX_CHARS_PER_BYTE = 2;

/** @brief Character used to separate bytes in formatted hex data */
inline constexpr QChar BYTE_SEPARATOR = ' ';

/** @brief Maximum length for message data input (16 hex chars + 7 spaces) */
inline constexpr int MAX_MESSAGE_DATA_INPUT_LENGTH = 23;  // "FF FF FF FF FF FF FF FF"

/** @brief Regular expression pattern for validating hex input with optional spaces */
inline const QRegularExpression HEX_VALIDATION_PATTERN(QStringLiteral("^[0-9A-Fa-f\\s]*$"));

/** @brief Default hex value for empty CAN ID field */
inline const std::string DEFAULT_CAN_ID_HEX = "000";

/** @brief Default hex value for empty data bytes */
inline const std::string DEFAULT_BYTE_HEX = "00";

// UI DIMENSIONS

/** @brief Fixed width for the mode selection sidebar in pixels */
inline constexpr int SIDEBAR_WIDTH = 120;

/** @brief Minimum width for the send message button in pixels */
inline constexpr int SEND_BUTTON_MIN_WIDTH = 160;

/** @brief Minimum height for the send message button in pixels */
inline constexpr int SEND_BUTTON_MIN_HEIGHT = 40;

/** @brief Height for the button container at the bottom in pixels */
inline constexpr int BUTTON_CONTAINER_HEIGHT = 80;

/** @brief Border thickness for active mode indicator in sidebar */
inline constexpr int SIDEBAR_ACTIVE_BORDER_WIDTH = 3;

// EDITOR CONFIGURATION

/** @brief Number of byte editors to display for CAN data payload */
inline constexpr int BYTE_EDITOR_COUNT = 8;

/** @brief Maximum length for general hex input strings */
inline constexpr int MAX_HEX_INPUT_LENGTH = 8;

/** @brief Number of decimal places for signal value editors */
inline constexpr int SIGNAL_VALUE_DECIMALS = 2;

/** @brief Minimum value for signal value spin box editor */
inline constexpr double SIGNAL_VALUE_MIN = -1e9;

/** @brief Maximum value for signal value spin box editor */
inline constexpr double SIGNAL_VALUE_MAX = 1e9;

}  // namespace Constants

}  // namespace Sending