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
