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

namespace FaultInjector::Constants {

/** @brief Label for fault injector card */
inline const QString FAULT_INJECTOR_TITLE = "Fault Injection";

/** @brief Sub title for fault injector card */
inline const QString FAULT_INJECTOR_SUBTITLE = "Dynamic Raw & Dbc based CAN Frame Fault Injection";

/** @brief Defines the maximum number of components shown in the faults list (in cell). */
inline constexpr int MAX_NUMBER_DYNAMIC_COMPONENTS = 2;

/** @brief Window name of the fault dialog. */
inline const QString FAULT_INJECTOR_WINDOW_NAME = "Edit Fault";

/** @brief Object name for the fault injector dialog, used for QSS scoping. */
inline const QString FAULT_INJECTOR_DIALOG_OBJECT_NAME = "faultInjectorDialog";

/** @brief Section label for the trigger configuration area. */
inline const QString FAULT_INJECTOR_DIALOG_TRIGGER_LABEL = "Trigger";

/** @brief Section label for the effect configuration area. */
inline const QString FAULT_INJECTOR_DIALOG_EFFECT_LABEL = "Effect";

/** @brief Section label for the strategy configuration area. */
inline const QString FAULT_INJECTOR_DIALOG_STRATEGY_LABEL = "Strategy";

/** @brief Section label for the mutation configuration area. */
inline const QString FAULT_INJECTOR_DIALOG_MUTATION_LABEL = "Mutation";

/** @brief Label for the add Raw fault button. */
inline const QString FAULT_INJECTOR_ADD_RAW_BUTTON_LABEL = "+ Raw";

/** @brief Label for the add DBC fault button. */
inline const QString FAULT_INJECTOR_ADD_DBC_BUTTON_LABEL = "+ DBC";

/** @brief Dialog title when adding a raw fault. */
inline const QString FAULT_INJECTOR_DIALOG_RAW_TITLE = "Add Raw Fault";

/** @brief Dialog title when adding a DBC fault. */
inline const QString FAULT_INJECTOR_DIALOG_DBC_TITLE = "Add DBC Fault";

/** @brief Icon path for the confirm button. */
inline const QString FAULT_INJECTOR_CONFIRM_ICON_PATH = ":/assets/icon/save.svg";

/** @brief Minimum width in pixels for the fault creation dialog */
inline constexpr int DIALOG_MIN_WIDTH = 700;

/** @brief Minimum height in pixels for the fault creation dialog */
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

}  // namespace FaultInjector::Constants