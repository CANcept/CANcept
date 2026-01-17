#pragma once
#include "core/theme/theme_manager.hpp"

/**
 * @brief Shortcut to access the global ThemeManager instance.
 *
 * Usage:
 *   auto color = THEME.colors().textPrimary;
 *   auto spacing = THEME.spacing().spacingMd;
 */
#define THEME Core::ThemeManager::getInstance()