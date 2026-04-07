#pragma once
#include <cassert>
#include <cstdint>

namespace FaultInjector {

/**
 * @struct BitFlipEffect
 * @brief This effect flips the corresponding bit based on 0 LSB, 7 MSB
 */
struct BitFlipEffect {
    uint8_t byteIndex;
    uint8_t bitIndex;

    /**
     * @brief Constructs a BitFlipEffect and makes sure the values are in range.
     * @param byte the byte of the bit to flip
     * @param bit the bit to flip inside that byte
     */
    BitFlipEffect(const uint8_t byte, const uint8_t bit) : byteIndex(byte), bitIndex(bit)
    {
        assert(bit < 8 && byte < 8);
    }
};

}  // namespace FaultInjector