#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace TestHelpers {

/**
 * @brief Minimal event types for testing the EventBroker in isolation.
 */

/** Single integer payload — minimal viable event. */
struct SimpleIntEvent {
    int value{0};
};

/** String payload — tests heap-allocated content round-trip. */
struct StringEvent {
    std::string message;
};

/** Multiple mixed-type fields — verifies that struct layout is preserved. */
struct MultiFieldEvent {
    int id{0};
    double factor{1.0};
    std::string label;
};

/** Zero-byte payload — marker / signal event with no data. */
struct EmptyEvent {
};

/** Large heap payload — ensures bulky data is not sliced or truncated. */
struct LargePayloadEvent {
    std::vector<uint8_t> data;
};

/** Nested struct — tests that the broker handles composite types. */
struct NestedEvent {
    SimpleIntEvent inner;
    std::string tag;
};

}  // namespace TestHelpers
