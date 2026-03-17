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
    int fontSizeXs = 10;
    int fontSizeSm = 12;
    int fontSizeMd = 14;
    int fontSizeLg = 18;

    // Icon Sizes
    int IconXs = 12;
    int IconSm = 20;
    int IconMd = 24;
    int IconLg = 48;

    // Heights
    int HeightXs = 18;
    int HeightSm = 32;
    int HeightMd = 48;
    int HeightLg = 120;
    int HeightXl = 350;

    // Widths
    int WidthXs = 120;
    int WidthSm = 240;
    int WidthMd = 480;
    int WidthLg = 600;
};

/**
 * @brief The normal spacing theme for the CanBusManager
 */
struct NormalSpacingTheme : public SpacingTheme {
};
}  // namespace Core