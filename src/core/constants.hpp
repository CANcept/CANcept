#pragma once
#include <QString>

/**
 * @namespace Core
 * @brief Centralized configuration for the application design system and assets.
 * * This file exists to eliminate "Magic Strings" from the codebase.
 */
namespace Core {

/** @namespace Assets - Virtual paths defined in the .qrc file */
namespace Assets {
inline const QString ThemePath = ":assets/qss/theme.qss";
}

/** * @namespace Theme
 * @brief Property names used in the selected qss theme for dynamic qproperty access.
 * * These strings must match the 'qproperty-NAME' defined in theme.qss.
 */
namespace Theme {
// Text Colors
inline constexpr auto TextPrimary = "text-primary";
inline constexpr auto TextSecondary = "text-secondary";
inline constexpr auto TextDisabled = "text-disabled";
inline constexpr auto TextOnPrimary = "text-on-primary";

// Surfaces
inline constexpr auto BgMain = "bg-main";
inline constexpr auto SurfacePrimary = "surface-primary";
inline constexpr auto SurfaceHover = "surface-hover";
inline constexpr auto SurfaceSelected = "surface-selected";

// Brand
inline constexpr auto ColorPrimary = "color-primary";
inline constexpr auto ColorPrimaryHover = "color-primary-hover";

// Borders
inline constexpr auto BorderSubtle = "border-subtle";
inline constexpr auto BorderStrong = "border-strong";
inline constexpr auto BorderThin = "border-thin";
inline constexpr auto BorderThick = "border-thick";

// Status
inline constexpr auto StatusSuccess = "status-success";
inline constexpr auto StatusError = "status-error";
inline constexpr auto StatusWarning = "status-warning";

// Spacing
inline constexpr auto SpacingXs = "spacing-xs";
inline constexpr auto SpacingSm = "spacing-sm";
inline constexpr auto SpacingMd = "spacing-md";
inline constexpr auto SpacingLg = "spacing-lg";
inline constexpr auto SpacingXl = "spacing-xl";

// Geometry
inline constexpr auto RadiusSm = "radius-sm";
inline constexpr auto RadiusMd = "radius-md";
inline constexpr auto RadiusLg = "radius-lg";

// Typography
inline constexpr auto WeightNormal = "font-weight-normal";
inline constexpr auto WeightMedium = "font-weight-medium";
inline constexpr auto WeightBold = "font-weight-bold";

inline constexpr auto SizeTextXs = "size-text-xs";
inline constexpr auto SizeTextSm = "size-text-sm";
inline constexpr auto SizeTextMd = "size-text-md";
inline constexpr auto SizeTextLg = "size-text-lg";
}  // namespace Theme
}  // namespace Core