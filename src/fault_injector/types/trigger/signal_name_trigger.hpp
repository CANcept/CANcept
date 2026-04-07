#pragma once
#include <string>
#include <utility>

namespace FaultInjector {

/**
 * @struct SignalNameTrigger
 * @brief This trigger activates based on the signal name of a message.
 */
struct SignalNameTrigger {
    std::string signal_name;

    /**
     * @brief Constructs a signal name trigger.
     * @param signal_name the signal name for the trigger
     */
    explicit SignalNameTrigger(std::string signal_name) : signal_name(std::move(signal_name)) {}
};

}  // namespace FaultInjector