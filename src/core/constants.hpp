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
inline const QString DbcFileTabIconPath = ":/assets/icon/dbc_file_tab.svg";
inline const QString UploadIconPath = ":/assets/icon/upload.svg";
inline const QString LoadNewIconPath = ":/assets/icon/load_new.svg";
inline const QString OverviewIconPath = ":/assets/icon/overview.svg";
inline const QString ECUsIconPath = ":/assets/icon/ecus.svg";
inline const QString MessagesIconPath = ":/assets/icon/messages.svg";
inline const QString SignalsIconPath = ":/assets/icon/signals.svg";
}  // namespace Assets
}  // namespace Core