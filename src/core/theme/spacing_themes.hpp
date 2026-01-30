#pragma once

namespace Core {

/**
 * @brief A simple struct defining application wide used spacing that can later be overriden.
 */
struct SpacingTheme {
    // Spacing
    int spacingXs = 4;
    int spacingSm = 8;
    int spacingMd = 12;
    int spacingLg = 16;
    int spacingXl = 24;

    // Border Radius
    int radiusXs = 6;
    int radiusSm = 8;
    int radiusMd = 16;
    int radiusLg = 24;

    // Border Width
    int borderThin = 1;
    int borderThick = 2;

    // Font Weights
    int fontWeightNormal = 400;
    int fontWeightMedium = 500;
    int fontWeightBold = 700;

    // Font Sizes
    int fontSizeBadge = 8;
    int fontSizeXs = 10;
    int fontSizeSm = 12;
    int fontSizeMd = 14;
    int fontSizeLg = 18;

    // Icon Sizes
    int iconXs = 12;
    int iconSm = 20;
    int IconLg = 48;

    // Item Card Spacing
    int itemCardWidth = 300;
    int itemCardHeight = 40;
    int itemCardGap = 2;
    int itemCardPadding = 12;
    int badgePadding = 5;

    // Badge sizes
    int badgeHeight = 18;
};

/**
 * @brief The normal spacing theme for the CanBusManager
 */
struct NormalSpacingTheme : public SpacingTheme {
};
}  // namespace Core