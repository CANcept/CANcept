#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace TestHelpers {

/**
 * @brief Minimal event types for testing the EventBroker in isolation.
 *
 * These are not application events — they cover a range of payload shapes
 * to verify that the broker correctly round-trips different data types.
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

/** Monotonic counter — lets tests verify delivery count and ordering. */
struct CounterEvent {
    int sequence{0};
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
