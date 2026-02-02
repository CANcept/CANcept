#pragma once
#include <QColor>

namespace Core {

/**
 * @brief A simple struct defining application wide used colors that can later be overriden.
 */
struct ColorTheme {
    // Text Colors
    QColor textPrimary = QColor(0, 0, 0);
    QColor textSecondary = QColor(0x5a5a5a);
    QColor textDisabled = QColor(0x9ca3af);
    QColor textOnPrimary = QColor(0xffffff);

    // Surfaces
    QColor surfaceMain = QColor(0xffffff);
    QColor surfacePrimary = QColor(0xf3f3f5);
    QColor surfaceSecondary = QColor(0xE2E2E2);
    QColor surfaceForeground = QColor(0, 0, 0);
    QColor surfaceHover = QColor(0xf3f4f6);
    QColor surfaceSelected = QColor(0xf3f4f6);

    // Standard Color
    QColor colorPrimary = QColor(0xE2E2E2);
    QColor colorPrimaryHover = QColor(0xB2B2B2);

    // Borders
    QColor borderSubtle = QColor(0, 0, 0, 0x19);
    QColor borderStrong = QColor(0, 0, 0, 0x32);

    // Status
    QColor statusSuccess = QColor(0x4C, 0xAF, 0x50, 0xFF);
    QColor statusError = QColor(0xF4, 0x43, 0x36, 0xFF);
    QColor statusWarning = QColor(0xFF, 0x98, 0x00, 0xFF);
    QColor statusRunning = QColor(0xFF, 0xC1, 0x07, 0xFF);
};

/**
 * @brief A light color theme of the CanBusManager
 */
struct LightTheme : public ColorTheme {
};

}  // namespace Core