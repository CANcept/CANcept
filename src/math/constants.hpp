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

#include <QtGlobal>

namespace Math::Constants {

/** @brief Icon for the "Add Variable" button in the variable selection dialog */
inline const QString ADD_VARIABLE_ICON_PATH = ":/assets/icon/math/add.svg";

/** @brief Icon for the "Confirm" button in the variable selection dialog */
inline const QString CONFIRM_ICON_PATH = ":/assets/icon/save.svg";

/** @brief Vertical padding multiplier relative to spacingXs for token cells */
inline constexpr double PAD_VERTICAL_FACTOR = 0.75;

/** @brief Margin multiplier relative to spacingSm around the expression canvas */
inline constexpr double MARGIN_FACTOR = 1.25;

/** @brief Padding multiplier for fraction bar spacing relative to spacingXs */
inline constexpr double BAR_PAD_FACTOR = 0.5;

/** @brief Timer interval in ms between spinner animation frames */
inline constexpr int SPIN_INTERVAL_MS = 30;

/** @brief Degrees the spinner arc advances per frame */
inline constexpr int SPIN_STEP_DEG = 12;

/** @brief Arc span in degrees for the spinner animation */
inline constexpr int SPIN_SPAN_DEG = 270;

/** @brief Pen width for spinner, check, and cross drawings */
inline constexpr qreal STROKE_WIDTH = 2.0;

/** @brief Inset margin for spinner and cross drawing rectangles */
inline constexpr qreal INDICATOR_MARGIN = 2.0;

/** @brief X ratio of the check mark start point */
inline constexpr qreal CHECK_RATIO_START_X = 0.15;

/** @brief X ratio of the check mark mid (bend) point */
inline constexpr qreal CHECK_RATIO_MID_X = 0.4;

/** @brief X ratio of the check mark end point */
inline constexpr qreal CHECK_RATIO_END_X = 0.85;

/** @brief Y ratio of the check mark start point */
inline constexpr qreal CHECK_RATIO_START_Y = 0.5;

/** @brief Y ratio of the check mark mid (bend) point */
inline constexpr qreal CHECK_RATIO_MID_Y = 0.75;

/** @brief Y ratio of the check mark end point */
inline constexpr qreal CHECK_RATIO_END_Y = 0.25;

/** @brief Minimum width in pixels for the variable selection dialog */
inline constexpr int DIALOG_MIN_WIDTH = 700;

/** @brief Minimum height in pixels for the scrollable variable list area */
inline constexpr int SCROLL_MIN_HEIGHT = 250;

/** @brief Allowed symbol shortcuts */
constexpr char WHITELIST_SHORTCUTS[] = {'+', '-', '*', '/'};

}  // namespace Math::Constants
