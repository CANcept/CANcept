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

namespace Manipulation::Constants {

/** @brief Label for manipulation card */
inline const QString MANIPULATION_TITLE = "Manipulation";

/** @brief Sub title for manipulation card */
inline const QString MANIPULATION_SUBTITLE = "Dynamic Raw & Dbc based CAN Frame Manipulation";

/** @brief Defines the maximum number of components shown in the manipulations list (in cell). */
inline constexpr int MAX_NUMBER_DYNAMIC_COMPONENTS = 2;

/** @brief Window name of the manipulation dialog. */
inline const QString MANIPULATION_WINDOW_NAME = "Edit Manipulation";

/** @brief Object name for the manipulation dialog, used for QSS scoping. */
inline const QString MANIPULATION_DIALOG_OBJECT_NAME = "manipulationDialog";

/** @brief Section label for the trigger configuration area. */
inline const QString MANIPULATION_DIALOG_TRIGGER_LABEL = "Trigger";

/** @brief Section label for the effect configuration area. */
inline const QString MANIPULATION_DIALOG_EFFECT_LABEL = "Effect";

/** @brief Section label for the strategy configuration area. */
inline const QString MANIPULATION_DIALOG_STRATEGY_LABEL = "Strategy";

/** @brief Section label for the mutation configuration area. */
inline const QString MANIPULATION_DIALOG_MUTATION_LABEL = "Mutation";

/** @brief Section label for the insert-message configuration area. */
inline const QString MANIPULATION_DIALOG_INSERT_LABEL = "Insert Message";

/** @brief Combo box entry that copies the frame which triggered the manipulation, instead
 * of a fixed, user-configured message. Selected by default. */
inline const QString MANIPULATION_DIALOG_INSERT_CURRENT_MESSAGE_LABEL = "Current Message";

/** @brief Hint shown in place of the signal editors when the insert strategy is set to
 * copy the triggering frame. */
inline const QString MANIPULATION_DIALOG_INSERT_CURRENT_MESSAGE_HINT =
    "A copy of the frame that triggered this manipulation will be inserted after the delay.";

/** @brief Label for the add Raw manipulation button. */
inline const QString MANIPULATION_ADD_RAW_BUTTON_LABEL = "+ Raw";

/** @brief Label for the add DBC manipulation button. */
inline const QString MANIPULATION_ADD_DBC_BUTTON_LABEL = "+ DBC";

/** @brief Dialog title when adding a raw manipulation. */
inline const QString MANIPULATION_DIALOG_RAW_TITLE = "Add Raw Manipulation";

/** @brief Dialog title when adding a DBC manipulation. */
inline const QString MANIPULATION_DIALOG_DBC_TITLE = "Add DBC Manipulation";

/** @brief Icon path for the confirm button. */
inline const QString MANIPULATION_CONFIRM_ICON_PATH = ":/assets/icon/save.svg";

/** @brief Minimum width in pixels for the manipulation creation dialog */
inline constexpr int DIALOG_MIN_WIDTH = 700;

/** @brief Minimum height in pixels for the manipulation creation dialog */
inline constexpr int SECTION_MIN_HEIGHT = 300;

/** @brief Object name for the CAN ID input in IdTriggerProvider. */
inline const QString PARAM_ID_INPUT = "idInput";

/** @brief Object name for the DLC spin box in DlcTriggerProvider. */
inline const QString PARAM_DLC_INPUT = "dlcInput";

/** @brief Object name for the probability spin box in RandomTriggerProvider. */
inline const QString PARAM_PROB_INPUT = "probInput";

/** @brief Object name for the signal name input in SignalNameTriggerProvider and
 * ValueSetEffectProvider. */
inline const QString PARAM_SIGNAL_NAME_INPUT = "signalNameInput";

/** @brief Object name for the byte index spin box in BitFlipEffectProvider. */
inline const QString PARAM_BYTE_INPUT = "byteInput";

/** @brief Object name for the bit index spin box in BitFlipEffectProvider. */
inline const QString PARAM_BIT_INPUT = "bitInput";

/** @brief Object name for the value spin box in ValueSetEffectProvider. */
inline const QString PARAM_VALUE_INPUT = "valueInput";

/** @brief Object name for the minimum value spin box in ClampEffectProvider. */
inline const QString PARAM_MIN_VALUE_INPUT = "minValueInput";

/** @brief Object name for the maximum value spin box in ClampEffectProvider. */
inline const QString PARAM_MAX_VALUE_INPUT = "maxValueInput";

/** @brief Object name for the amplitude spin box in NoiseEffectProvider. */
inline const QString PARAM_AMPLITUDE_INPUT = "amplitudeInput";

/** @brief Object name for the delay spin box in DelayedStrategyProvider. */
inline const QString PARAM_DELAY_INPUT = "delayInput";

/** @brief Object name for the threshold spin box in SignalThresholdTriggerProvider. */
inline const QString PARAM_THRESHOLD_INPUT = "thresholdInput";

/** @brief Object name for the comparison combo box in SignalThresholdTriggerProvider. */
inline const QString PARAM_IS_GREATER_INPUT = "isGreaterInput";

}  // namespace Manipulation::Constants