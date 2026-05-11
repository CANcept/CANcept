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
#include <chrono>
#include <cstddef>

namespace Logging::Constants {
inline const QString LOGGING_ICON_PATH = ":/assets/icon/logging/logging_tab.svg";
inline const QString START_ICON_PATH = ":/assets/icon/logging/start_icon.svg";
inline const QString STOP_ICON_PATH = ":/assets/icon/logging/stop_icon.svg";

/** @brief Path to the info Logo. */
inline const QString INFO_ICON_PATH = ":/assets/icon/logging/info.svg";

/** @brief Path to the data Logo. */
inline const QString DATA_ICON_PATH = ":/assets/icon/logging/database.svg";

inline const std::string MODULE_IDENTIFIER = "LoggingComponent";

inline constexpr int BUTTON_MIN_HEIGHT = 32;
inline constexpr int BUTTON_MIN_WIDTH = 80;

/** @brief Minimum width of the message selection dialog. */
inline constexpr int DIALOG_MIN_WIDTH = 400;
/** @brief Initial height of the message selection dialog. */
inline constexpr int DIALOG_START_HEIGHT = 500;

/** @brief Window title of the message selection dialog. */
inline const QString DIALOG_TITLE = "Select Messages to Log";
/** @brief Label for the raw log type toggle option. */
inline const QString DIALOG_RAW_LABEL = "Raw";
/** @brief Label for the DBC-based log type toggle option. */
inline const QString DIALOG_DBC_LABEL = "DBC based";
/** @brief Title of the message card section in the selection dialog. */
inline const QString DIALOG_MESSAGES_CARD_TITLE = "Messages";
/** @brief Tooltip shown on the per-message selection checkbox. */
inline const QString DIALOG_MESSAGE_CHECKBOX_TOOLTIP = "Select message for logging";
/** @brief Placeholder text shown in the messages card when raw logging is selected. */
inline const QString DIALOG_RAW_PLACEHOLDER =
    "All CAN frames will be captured — no filtering needed.";

inline constexpr std::size_t WORKER_RING_CAPACITY = 1 << 16;
inline constexpr auto WORKER_FLUSH_INTERVAL = std::chrono::milliseconds(500);

/** @brief Number of records loaded per page in the detail view. */
inline constexpr int DETAIL_PAGE_SIZE = 200;

/** @brief Title of the session info card in the detail view. */
inline const QString DETAIL_INFO_CARD_TITLE = "Session Info";
/** @brief Title of the data table card in the detail view. */
inline const QString DETAIL_DATA_CARD_TITLE = "Data";

/** @brief Label key for session ID in the info card. */
inline const QString DETAIL_LABEL_SESSION_ID = "Session ID";
/** @brief Label key for timestamp in the info card. */
inline const QString DETAIL_LABEL_TIMESTAMP = "Timestamp";
/** @brief Label key for duration in the info card. */
inline const QString DETAIL_LABEL_DURATION = "Duration";
/** @brief Label key for message count in the info card. */
inline const QString DETAIL_LABEL_MESSAGES = "Messages";

/** @brief DateTime format string used in the detail info card. */
inline const QString DETAIL_DATETIME_FORMAT = "dd.MM.yyyy HH:mm:ss";

/** @brief Label for the previous-page button in the detail view. */
inline const QString DETAIL_PREV_BTN_LABEL = "<";
/** @brief Label for the next-page button in the detail view. */
inline const QString DETAIL_NEXT_BTN_LABEL = ">";
/** @brief Label for the back button in the detail view. */
inline const QString DETAIL_BACK_BTN_LABEL = "Back";
}  // namespace Logging::Constants
