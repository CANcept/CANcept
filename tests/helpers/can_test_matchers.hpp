#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "core/dto/can_dto.hpp"

namespace TestHelpers {

/**
 * @brief Custom matcher for Core::RawCanMessage ID.
 *
 * Usage:
 * @code
 *   EXPECT_THAT(rawMsg, HasRawCanId(0x123));
 * @endcode
 */
MATCHER_P(HasRawCanId, expectedId, "")
{
    if (arg.messageId != expectedId)
    {
        *result_listener << "has CAN ID 0x" << std::hex << arg.messageId << ", expected 0x"
                         << expectedId;
        return false;
    }
    return true;
}

/**
 * @brief Custom matcher for Core::RawCanMessage DLC.
 *
 * Usage:
 * @code
 *   EXPECT_THAT(rawMsg, HasRawDlc(4));
 * @endcode
 */
MATCHER_P(HasRawDlc, expectedDlc, "")
{
    if (arg.dlc != expectedDlc)
    {
        *result_listener << "has DLC " << static_cast<int>(arg.dlc) << ", expected "
                         << static_cast<int>(expectedDlc);
        return false;
    }
    return true;
}

/**
 * @brief Custom matcher for Core::RawCanMessage data content.
 *
 * Handles signed/unsigned char comparison properly.
 *
 * Usage:
 * @code
 *   EXPECT_THAT(rawMsg, HasRawData({0xAA, 0xBB, 0xCC}));
 * @endcode
 */
MATCHER_P(HasRawData, expectedData, "")
{
    const std::vector<uint8_t> expected(expectedData.begin(), expectedData.end());

    if (arg.dlc != expected.size())
    {
        *result_listener << "has DLC " << static_cast<int>(arg.dlc) << ", expected "
                         << expected.size();
        return false;
    }

    for (size_t i = 0; i < expected.size(); ++i)
    {
        // Cast both to unsigned for proper comparison
        auto actualByte = static_cast<uint8_t>(arg.data[i]);
        if (actualByte != expected[i])
        {
            *result_listener << "has data byte [" << i << "] = 0x" << std::hex
                             << static_cast<int>(actualByte) << ", expected 0x"
                             << static_cast<int>(expected[i]);
            return false;
        }
    }
    return true;
}

/**
 * @brief Custom matcher for complete Core::RawCanMessage verification.
 *
 * Usage:
 * @code
 *   EXPECT_THAT(rawMsg, IsRawCanMessage(0x123, {0x01, 0x02, 0x03}));
 * @endcode
 */
MATCHER_P2(IsRawCanMessage, expectedId, expectedData, "")
{
    // Check ID
    if (arg.messageId != expectedId)
    {
        *result_listener << "has CAN ID 0x" << std::hex << arg.messageId << ", expected 0x"
                         << expectedId;
        return false;
    }

    // Check data
    const std::vector<uint8_t> expected(expectedData.begin(), expectedData.end());

    if (arg.dlc != expected.size())
    {
        *result_listener << "has DLC " << static_cast<int>(arg.dlc) << ", expected "
                         << expected.size();
        return false;
    }

    for (size_t i = 0; i < expected.size(); ++i)
    {
        if (auto actualByte = static_cast<uint8_t>(arg.data[i]); actualByte != expected[i])
        {
            *result_listener << "has data byte [" << i << "] = 0x" << std::hex
                             << static_cast<int>(actualByte) << ", expected 0x"
                             << static_cast<int>(expected[i]);
            return false;
        }
    }

    return true;
}

}  // namespace TestHelpers
