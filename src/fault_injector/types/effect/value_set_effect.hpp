#pragma once
#include <string>
#include <utility>

namespace FaultInjector {

/**
 * @struct ValueSetEffect
 * @brief This effect sets the value of a certain signal.
 */
struct ValueSetEffect {
    std::string signal_name;
    double value;

    explicit ValueSetEffect(std::string signal_name, const double value)
        : signal_name(std::move(signal_name)), value(value)
    {
    }
};

}  // namespace FaultInjector