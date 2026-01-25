#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Sending {

/**
 * @namespace Constants
 * @brief Module-level constants for CAN message sending configuration.
 */
namespace Constants {

/** @brief Icon used for the Tab */
inline const QString SENDING_ICON_PATH = ":/assets/icon/sending/sending.svg";

/** @brief Icon used for the send button */
inline const QString SEND_BUTTON_ICON_PATH = ":/assets/icon/sending/send.svg";

/** @brief Icon used for the raw sub Tab */
inline const QString RAW_SENDING_ICON_PATH = ":/assets/icon/sending/raw_based_send_icon.svg";

/** @brief Icon used for the dbc sub Tab */
inline const QString DBC_SENDING_ICON_PATH = ":/assets/icon/sending/dbc_based_send_icon.svg";

/** @brief Icon used for the dbc configuration tabs */
inline const QString CONFIGURATION_ICON_PATH = ":/assets/icon/sending/configuration.svg";

/** @brief Icon used for the dbc configuration tabs */
inline const QString CAN_FRAME_ICON_PATH = ":/assets/icon/sending/can_frame.svg";

/** @brief Identifier used by the module to communicate with the system */
inline const std::string MODULE_IDENTIFIER = "SendingComponent";

/** @brief Title shown in the tab bar */
inline const QString TAB_TITLE = "Sending";

/** @brief Maximum number of data bytes in a standard CAN frame */
inline constexpr uint8_t MAX_CAN_DLC = 8;

/** @brief Maximum Standard CAN ID (11-bit) */
inline constexpr uint32_t MAX_STANDARD_CAN_ID = 0x7FF;

/** @brief Maximum Extended CAN ID (29-bit) */
inline constexpr uint32_t MAX_EXTENDED_CAN_ID = 0x1FFFFFFF;

/** @brief Default list of CAN interfaces to populate in device selectors */
inline const std::vector<std::string> DEFAULT_CAN_DEVICES = {"vcan0", "can0", "can1"};

/** @brief Standard CAN bitrates supported by most hardware */
inline const std::vector<uint32_t> STANDARD_BITRATES = {125000, 250000, 500000, 1000000};

/** @brief Default cyclic transmission interval in milliseconds */
inline constexpr int DEFAULT_CYCLE_INTERVAL_MS = 100;

/** @brief Minimum allowed cyclic interval (to prevent system overload) */
inline constexpr int MIN_CYCLE_INTERVAL_MS = 10;

/** @brief Maximum allowed cyclic interval */
inline constexpr int MAX_CYCLE_INTERVAL_MS = 10000;

/** @brief Number of byte editors to display for CAN data payload */
inline constexpr int BYTE_EDITOR_COUNT = 8;

/** @brief Default hex string for empty CAN ID field */
inline const std::string DEFAULT_CAN_ID_HEX = "000";

/** @brief Default hex string for empty data bytes */
inline const std::string DEFAULT_BYTE_HEX = "00";

/** @brief Default placeholder for CAN ID input with 0x prefix */
inline const QString DEFAULT_CAN_ID_PLACEHOLDER = "0x 1A2B";

/** @brief Default placeholder for message data input */
inline const QString DEFAULT_MESSAGE_DATA_PLACEHOLDER = "01 02 03 04 05 06 07 08";

/** @brief Maximum length for hex input strings  */
inline constexpr int MAX_HEX_INPUT_LENGTH = 8;

/** @brief Regex pattern for validating hex input */
inline const std::string HEX_VALIDATION_PATTERN = "^[0-9A-Fa-f]+$";

}  // namespace Constants

}  // namespace Sending