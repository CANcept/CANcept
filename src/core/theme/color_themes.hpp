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

/**
 * @brief A dark color theme of the CanBusManager
 */
struct DarkTheme : public ColorTheme {
    DarkTheme()
    {
        // Text Colors
        textPrimary = QColor(0xe0e0e0);
        textSecondary = QColor(0x9e9e9e);
        textDisabled = QColor(0x616161);
        textOnPrimary = QColor(0, 0, 0);

        // Surfaces
        surfaceMain = QColor(0x1e, 0x1e, 0x2e);
        surfacePrimary = QColor(0x2a, 0x2a, 0x3c);
        surfaceSecondary = QColor(0x38, 0x38, 0x4c);
        surfaceForeground = QColor(0xe0e0e0);
        surfaceHover = QColor(0x33, 0x33, 0x46);
        surfaceSelected = QColor(0x33, 0x33, 0x46);

        // Standard Color
        colorPrimary = QColor(0x38, 0x38, 0x4c);
        colorPrimaryHover = QColor(0x4a, 0x4a, 0x60);

        // Borders
        borderSubtle = QColor(255, 255, 255, 0x19);
        borderStrong = QColor(255, 255, 255, 0x32);

        // Status (keep readable on dark background)
        statusSuccess = QColor(0x66, 0xBB, 0x6A);
        statusError = QColor(0xEF, 0x53, 0x50);
        statusWarning = QColor(0xFF, 0xA7, 0x26);
        statusRunning = QColor(0xFF, 0xCA, 0x28);
    }
};

/**
 * @brief An aqua color theme with cool and awesome tones
 */
struct AquaTheme : public ColorTheme {
    AquaTheme()
    {
        // Text Colors
        textPrimary = QColor(0x0f, 0x1f, 0x3d);
        textSecondary = QColor(0x4a, 0x5a, 0x7a);
        textDisabled = QColor(0x9c, 0xa3, 0xaf);
        textOnPrimary = QColor(0xff, 0xff, 0xff);

        // Surfaces
        surfaceMain = QColor(0xf0, 0xf4, 0xf8);
        surfacePrimary = QColor(0xe3, 0xec, 0xf5);
        surfaceSecondary = QColor(0xd1, 0xdf, 0xed);
        surfaceForeground = QColor(0x1e, 0x40, 0x78);
        surfaceHover = QColor(0xe8, 0xf0, 0xf8);
        surfaceSelected = QColor(0xd6, 0xe4, 0xf5);

        // Standard Color
        colorPrimary = QColor(0x2c, 0x5f, 0xa3);
        colorPrimaryHover = QColor(0x1e, 0x4a, 0x7f);

        // Borders
        borderSubtle = QColor(0x2c, 0x5f, 0xa3, 0x19);
        borderStrong = QColor(0x2c, 0x5f, 0xa3, 0x32);

        // Status
        statusSuccess = QColor(0x4C, 0xAF, 0x50);
        statusError = QColor(0xF4, 0x43, 0x36);
        statusWarning = QColor(0xFF, 0x98, 0x00);
        statusRunning = QColor(0xFF, 0xC1, 0x07);
    }
};

/**
 * @brief A maroon theme with warm majestic gold and red tones
 */
struct MaroonTheme : public ColorTheme {
    MaroonTheme()
    {
        // Text Colors
        textPrimary = QColor(0x3a, 0x0f, 0x0f);
        textSecondary = QColor(0x6d, 0x4c, 0x41);
        textDisabled = QColor(0x9c, 0x8a, 0x7f);
        textOnPrimary = QColor(0xff, 0xff, 0xff);

        // Surfaces (warm beige/cream tones)
        surfaceMain = QColor(0xf8, 0xf4, 0xed);
        surfacePrimary = QColor(0xf5, 0xe6, 0xd3);
        surfaceSecondary = QColor(0xed, 0xd5, 0xb8);
        surfaceForeground = QColor(0x80, 0x00, 0x20);
        surfaceHover = QColor(0xf9, 0xef, 0xe0);
        surfaceSelected = QColor(0xf7, 0xe4, 0xce);

        // Standard Color (maroon/burgundy)
        colorPrimary = QColor(0x80, 0x00, 0x20);
        colorPrimaryHover = QColor(0x5f, 0x00, 0x18);

        // Borders (warm brown tones)
        borderSubtle = QColor(0x80, 0x00, 0x20, 0x19);
        borderStrong = QColor(0x80, 0x00, 0x20, 0x32);

        // Status (warm tones)
        statusSuccess = QColor(0x6B, 0x8E, 0x23);
        statusError = QColor(0xDC, 0x14, 0x3C);
        statusWarning = QColor(0xFF, 0x8C, 0x00);
        statusRunning = QColor(0xFF, 0xD7, 0x00);
    }
};

/**
 * @brief A Dracula-inspired theme with dark purple tones :)
 */
struct DraculaTheme : public ColorTheme {
    DraculaTheme()
    {
        // Text Colors (Dracula fg colors)
        textPrimary = QColor(0xf8, 0xf8, 0xf2);
        textSecondary = QColor(0x6f, 0x72, 0x88);
        textDisabled = QColor(0x44, 0x47, 0x5a);
        textOnPrimary = QColor(0xf8, 0xf8, 0xf2);

        // Surfaces (Dracula background colors)
        surfaceMain = QColor(0x28, 0x29, 0x36);
        surfacePrimary = QColor(0x34, 0x35, 0x46);
        surfaceSecondary = QColor(0x44, 0x47, 0x5a);
        surfaceForeground = QColor(0xbd, 0x93, 0xf9);  // Purple
        surfaceHover = QColor(0x3e, 0x3f, 0x52);
        surfaceSelected = QColor(0x44, 0x47, 0x5a);

        // Standard Color (Dracula purple)
        colorPrimary = QColor(0xbd, 0x93, 0xf9);
        colorPrimaryHover = QColor(0xa0, 0x7a, 0xe0);

        // Borders (subtle purple tint)
        borderSubtle = QColor(0xbd, 0x93, 0xf9, 0x19);
        borderStrong = QColor(0xbd, 0x93, 0xf9, 0x32);

        // Status (Dracula colors)
        statusSuccess = QColor(0x50, 0xfa, 0x7b);
        statusError = QColor(0xff, 0x55, 0x55);
        statusWarning = QColor(0xff, 0xb8, 0x6c);
        statusRunning = QColor(0xf1, 0xfa, 0x8c);
    }
};

}  // namespace Core