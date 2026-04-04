#pragma once

#include <chrono>
#include <memory>
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
    explicit TimeVariable(const TimeUnit unit)
        : m_unit(unit),
          m_factor(factorFor(unit)),
          m_value(std::make_shared<double>(0.0)),
          m_start(std::chrono::steady_clock::now())
    {
    }

    auto configKey() const -> std::string override
    {
        return "time:" + unitSuffix(m_unit);
    }

    auto displayName() const -> std::string override
    {
        switch (m_unit)
        {
            case TimeUnit::Seconds:
                return "Time (s)";
            case TimeUnit::Milliseconds:
                return "Time (ms)";
            case TimeUnit::Nanoseconds:
                return "Time (ns)";
        }
        return "Time";
    }

    auto ptr() -> double* override
    {
        return m_value.get();
    }

    auto sharedPtr() -> std::shared_ptr<double> override
    {
        return m_value;
    }

    void update() override
    {
        const auto elapsed = std::chrono::steady_clock::now() - m_start;
        *m_value = std::chrono::duration<double, std::nano>(elapsed).count() * m_factor;
    }

    static auto unitSuffix(const TimeUnit unit) -> std::string
    {
        switch (unit)
        {
            case TimeUnit::Seconds:
                return "seconds";
            case TimeUnit::Milliseconds:
                return "milliseconds";
            case TimeUnit::Nanoseconds:
                return "nanoseconds";
        }
        return "seconds";
    }

    [[nodiscard]] auto unit() const -> TimeUnit
    {
        return m_unit;
    }

    static auto unitFromSuffix(const std::string& suffix) -> TimeUnit
    {
        if (suffix == "milliseconds") return TimeUnit::Milliseconds;
        if (suffix == "nanoseconds") return TimeUnit::Nanoseconds;
        return TimeUnit::Seconds;
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

    TimeUnit m_unit;
    double m_factor;
    std::shared_ptr<double> m_value;
    std::chrono::steady_clock::time_point m_start;
};

}  // namespace Math
