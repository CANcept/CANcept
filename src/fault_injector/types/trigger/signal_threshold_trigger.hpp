#pragma once
#include <string>
#include <utility>

namespace FaultInjector {

/**
 * @struct SignalThresholdTrigger
 * @brief This trigger activates based on the signal name and if that signal reached a certain
 * threshold of a message.
 */
struct SignalThresholdTrigger {
    std::string signal_name;
    double threshold;
    bool isGreater = true;

    /**
     * @brief Constructs a signal name trigger.
     * @param signal_name the signal name for the trigger
     * @param threshold the threshold for the trigger
     * @param isGreater if the operation is greater then or less then
     */
    explicit SignalThresholdTrigger(std::string signal_name, const double threshold,
                                    const bool isGreater = false)
        : signal_name(std::move(signal_name)), threshold(threshold), isGreater(isGreater)
    {
    }
};

}  // namespace FaultInjector