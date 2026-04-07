#pragma once
#include <cassert>
#include <cstdint>

namespace FaultInjector {

/**
 * @struct IDTrigger
 * @brief This trigger activates when the specific id is used in the message.
 */
struct IDTrigger {
    uint16_t id;

    /**
     * @brief Checks that the id is between 0 and 7FF.
     * @param id the message id for the trigger
     */
    explicit IDTrigger(const uint16_t id) : id(id)
    {
        assert(id <= 0x7FF);
    }
};

}  // namespace FaultInjector