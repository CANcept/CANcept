#pragma once
#include <QString>

namespace FaultInjector::Constants {

/** @brief Label for fault injector card */
inline const QString FAULT_INJECTOR_TITLE = "Fault Injection";

/** @brief Sub title for fault injector card */
inline const QString FAULT_INJECTOR_SUBTITLE = "Dynamic Raw & Dbc based CAN Frame Fault Injection";

/** @brief Defines the maximum number of components shown in the faults list (in cell). */
inline constexpr int MAX_NUMBER_DYNAMIC_COMPONENTS = 2;

}  // namespace FaultInjector::Constants