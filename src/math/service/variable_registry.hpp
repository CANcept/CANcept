#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../types/variables/i_variable.hpp"

namespace Math
{

/**
 * @brief Owns all variables available to value functions and updates them each cycle.
 */
class VariableRegistry
{
public:
    void add(std::unique_ptr<IVariable> variable);

    /** @brief Resolves a variable name to its double pointer, returns nullptr if not found. */
    double* resolve(const std::string& name) const;

    /** @brief Updates all registered variables, called once per cycle before evaluation. */
    void updateAll() const;

    const std::vector<std::unique_ptr<IVariable>>& variables() const;

private:
    std::vector<std::unique_ptr<IVariable>> m_variables;
};

}  // namespace Math
