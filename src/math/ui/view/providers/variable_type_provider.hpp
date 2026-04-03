#pragma once

#include <QWidget>
#include <memory>
#include <string>

#include "math/types/variables/i_variable.hpp"

namespace Math {

/**
 * @brief Factory interface for a variable type.
 */
class IVariableTypeProvider
{
   public:
    virtual ~IVariableTypeProvider() = default;

    /**
     * @brief Display name shown in the type dropdown.
     */
    virtual auto typeName() const -> QString = 0;

    /**
     * @brief Creates the type-specific options widget. Ownership passes to caller.
     */
    virtual auto createOptionsWidget(QWidget* parent) -> QWidget* = 0;

    /**
     * @brief Builds the config key from the current state of the options widget.
     *
     * Returns an empty string if the options are incomplete or invalid.
     */
    virtual auto configKey(const QWidget* optionsWidget) const -> std::string = 0;

    /**
     * @brief Human-readable display name derived from current options.
     */
    virtual auto displayName(const QWidget* optionsWidget) const -> std::string = 0;

    /**
     * @brief Creates an IVariable instance from the current options widget state.
     */
    virtual auto createVariable(const QWidget* optionsWidget) const
        -> std::unique_ptr<IVariable> = 0;
};

}  // namespace Math
