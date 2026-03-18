#pragma once

#include <string>

namespace Math
{

/**
 * @brief Contract for a variable whose value is updated each cycle before evaluation.
 */
class IVariable
{
public:
    virtual ~IVariable() = default;

    /** @brief Single-token identifier used in expressions and registered with exprtk. */
    virtual const std::string& symbol() const = 0;

    /** @brief Human-readable label shown in the UI. */
    virtual const std::string& name() const = 0;

    virtual double* ptr() = 0;
    virtual void update() = 0;
};

}  // namespace Math
