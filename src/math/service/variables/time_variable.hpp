#pragma once

#include <chrono>
#include <string>

#include "math/types/variables/i_variable.hpp"

namespace Math {

/**
 * @brief Time unit used to scale the elapsed value written to the expression variable.
 */
enum class TimeUnit { Seconds, Milliseconds, Nanoseconds };

/**
 * @brief Variable that provides elapsed time since construction in a chosen unit.
 */
class TimeVariable final : public IVariable
{
   public:
    TimeVariable(std::string symbol, std::string name, const TimeUnit unit)
        : m_symbol(std::move(symbol)),
          m_name(std::move(name)),
          m_factor(factorFor(unit)),
          m_start(std::chrono::steady_clock::now())
    {
    }

    const std::string& symbol() const override
    {
        return m_symbol;
    }
    const std::string& name() const override
    {
        return m_name;
    }
    double* ptr() override
    {
        return &m_value;
    }

    void update() override
    {
        const auto elapsed = std::chrono::steady_clock::now() - m_start;
        m_value = std::chrono::duration<double, std::nano>(elapsed).count() * m_factor;
    }

   private:
    static constexpr double NANOS_TO_SECONDS = 1e-9;
    static constexpr double NANOS_TO_MILLIS = 1e-6;
    static constexpr double NANOS_TO_NANOS = 1.0;

    static double factorFor(const TimeUnit unit)
    {
        switch (unit)
        {
            case TimeUnit::Seconds:
                return NANOS_TO_SECONDS;
            case TimeUnit::Milliseconds:
                return NANOS_TO_MILLIS;
            case TimeUnit::Nanoseconds:
                return NANOS_TO_NANOS;
        }
        return NANOS_TO_SECONDS;
    }

    std::string m_symbol;
    std::string m_name;
    double m_factor;
    double m_value = 0.0;
    std::chrono::steady_clock::time_point m_start;
};

}  // namespace Math
