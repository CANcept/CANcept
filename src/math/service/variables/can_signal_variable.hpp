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

#include <cstdint>
#include <memory>
#include <string>

#include "math/types/variables/i_variable.hpp"

namespace Math {

/**
 * @brief Variable backed by a decoded CAN signal value, updated externally by the registry.
 */
class CanSignalVariable final : public IVariable
{
   public:
    CanSignalVariable(uint32_t messageId, std::string signalName, std::string messageName)
        : m_messageId(messageId),
          m_signalName(std::move(signalName)),
          m_messageName(std::move(messageName)),
          m_value(std::make_shared<double>(0.0))
    {
    }

    auto configKey() const -> std::string override
    {
        return "signal:" + std::to_string(m_messageId) + ":" + m_signalName;
    }

    void reset() override {};

    auto displayName() const -> std::string override
    {
        return m_messageName + "." + m_signalName;
    }

    auto ptr() -> double* override
    {
        return m_value.get();
    }

    auto sharedPtr() -> std::shared_ptr<double> override
    {
        return m_value;
    }

    void update() override {}

    void setValue(const double v)
    {
        *m_value = v;
    }

    [[nodiscard]] auto messageId() const -> uint32_t
    {
        return m_messageId;
    }

    [[nodiscard]] auto signalName() const -> const std::string&
    {
        return m_signalName;
    }

   private:
    uint32_t m_messageId;
    std::string m_signalName;
    std::string m_messageName;
    std::shared_ptr<double> m_value;
};

}  // namespace Math
