#pragma once
#include <QString>

/**
 * @namespace Core
 * @brief Centralized configuration for the application constants and assets.
 * * This file exists to eliminate "Magic Strings" from the codebase.
 */
namespace Core {

/** @namespace Assets - Virtual paths defined in the .qrc file */
namespace Assets {
inline const QString CanBusIconPath = ":/assets/icon/can_bus.svg";
}  // namespace Assets

}  // namespace Core