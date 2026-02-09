#pragma once

#include <string>
#include <utility>

#include "event.hpp"
#include "settings_event.hpp"

namespace Core {

/** @brief Event to get available theme options from providers. */
struct GetAvailableThemesEvent final : public SelectProviderOptionEvent {
    explicit GetAvailableThemesEvent(std::list<SelectOption>* options)
        : SelectProviderOptionEvent(options){};
};

/** @brief Published when the user selects a different theme. */
struct ThemeChangeEvent final : public Event {
    std::string themeName;

    explicit ThemeChangeEvent(std::string themeName) : themeName(std::move(themeName)){};
};

}  // namespace Core
