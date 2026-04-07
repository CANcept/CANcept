#pragma once
#include <cassert>

namespace FaultInjector {

/**
 * @struct RandomTrigger
 * @brief This trigger activates based on the given probability.
 */
struct RandomTrigger {
    float probability;

    /**
     * @brief Checks that the chance is between 0 and 1.
     * @param chance the chance to hit the trigger
     */
    explicit RandomTrigger(const float chance) : probability(chance)
    {
        assert(chance >= 0.0f && chance <= 1.0f);
    }
};

}  // namespace FaultInjector
