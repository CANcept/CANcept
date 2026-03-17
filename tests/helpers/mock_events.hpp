/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
