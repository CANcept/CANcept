#include "math/service/variable_registry.hpp"

#include <algorithm>

namespace Math {

void VariableRegistry::add(std::unique_ptr<IVariable> variable)
{
    m_variables.push_back(std::move(variable));
}

double* VariableRegistry::resolve(const std::string& name) const
{
    const auto it =
        std::ranges::find_if(m_variables, [&](const auto& v) { return v->symbol() == name; });
    return it != m_variables.end() ? (*it)->ptr() : nullptr;
}

void VariableRegistry::updateAll() const
{
    for (auto& variable : m_variables) variable->update();
}

const std::vector<std::unique_ptr<IVariable>>& VariableRegistry::variables() const
{
    return m_variables;
}

}  // namespace Math
