#pragma once
#include <cassert>
#include <cstdint>

namespace FaultInjector {
/**
 * @struct DLCTrigger
 * @brief This trigger activates when the specific dlc is used in the message.
 */
struct DLCTrigger {
    uint8_t dlc;

    /**
     * @brief Checks that the dlc is between 0 and 8.
     * @param dlc the dlc for the trigger
     */
    explicit DLCTrigger(const uint8_t dlc) : dlc(dlc)
    {
        assert(dlc <= 8);
    }
};

}  // namespace FaultInjector