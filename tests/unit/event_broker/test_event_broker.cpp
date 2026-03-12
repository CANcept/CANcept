#include <gtest/gtest.h>

#include <atomic>
#include <functional>
#include <string>
#include <vector>

#include "event_broker/event_broker.hpp"
#include "tests/helpers/mock_events.hpp"

using namespace TestHelpers;

struct DeliveryScenario {
    std::string name;
    int subscriberCount;
    int publishCount;
    int releasedCount;  // connections released before publishing
    int expectedCalls;
    std::function<Core::Connection(EventBroker::EventBroker&, std::function<void()>)>
        makeSubscription;
    std::function<void(EventBroker::EventBroker&)> doPublish;
};

class EventBrokerDeliveryTest : public ::testing::TestWithParam<DeliveryScenario>
{
   protected:
    EventBroker::EventBroker broker;
};

TEST_P(EventBrokerDeliveryTest, CallCountMatchesExpectation)
{
    const auto& p = GetParam();
    std::atomic<int> callCount{0};
    std::vector<Core::Connection> connections;
    connections.reserve(p.subscriberCount);

    for (int i = 0; i < p.subscriberCount; ++i)
    {
        connections.push_back(p.makeSubscription(broker, [&] { ++callCount; }));
    }

    for (int i = 0; i < p.releasedCount; ++i)
    {
        connections[i].release();
    }

    for (int i = 0; i < p.publishCount; ++i)
    {
        p.doPublish(broker);
    }

    EXPECT_EQ(callCount.load(), p.expectedCalls);
}

INSTANTIATE_TEST_SUITE_P(
    BrokerDeliveryScenarios, EventBrokerDeliveryTest,
    ::testing::Values(
        // Basic delivery: one subscriber receives one publish
        DeliveryScenario{"OneSub_OnePub", 1, 1, 0, 1,
                         [](EventBroker::EventBroker& b, std::function<void()> cb) {
                             return b.subscribe<SimpleIntEvent>(
                                 [cb](const SimpleIntEvent&) { cb(); });
                         },
                         [](EventBroker::EventBroker& b) { b.publish(SimpleIntEvent{}); }},

        // Fan-out: N subscribers × M publishes = N×M total calls
        DeliveryScenario{"FiveSubs_FivePubs", 5, 5, 0, 25,
                         [](EventBroker::EventBroker& b, std::function<void()> cb) {
                             return b.subscribe<SimpleIntEvent>(
                                 [cb](const SimpleIntEvent&) { cb(); });
                         },
                         [](EventBroker::EventBroker& b) { b.publish(SimpleIntEvent{}); }},

        // Partial release: only unreleased connections receive
        DeliveryScenario{"ThreeSubs_TwoReleased_OnePub", 3, 1, 2, 1,
                         [](EventBroker::EventBroker& b, std::function<void()> cb) {
                             return b.subscribe<SimpleIntEvent>(
                                 [cb](const SimpleIntEvent&) { cb(); });
                         },
                         [](EventBroker::EventBroker& b) { b.publish(SimpleIntEvent{}); }},

        // All released: zero calls regardless of publish count
        DeliveryScenario{"ThreeSubs_AllReleased_FivePubs", 3, 5, 3, 0,
                         [](EventBroker::EventBroker& b, std::function<void()> cb) {
                             return b.subscribe<StringEvent>([cb](const StringEvent&) { cb(); });
                         },
                         [](EventBroker::EventBroker& b) { b.publish(StringEvent{}); }},

        // No subscribers: publish does not crash and call count is zero
        DeliveryScenario{"NoSubs_TenPubs", 0, 10, 0, 0,
                         [](EventBroker::EventBroker& b, std::function<void()> cb) {
                             return b.subscribe<EmptyEvent>([cb](const EmptyEvent&) { cb(); });
                         },
                         [](EventBroker::EventBroker& b) { b.publish(EmptyEvent{}); }}),
    [](const ::testing::TestParamInfo<DeliveryScenario>& info) { return info.param.name; });

/**
 * @brief Each scenario uses an IIFE to create shared state (via shared_ptr) captured
 * by all three lambdas, so subscribe, publish, and verify refer to the same storage.
 */
struct ContentScenario {
    std::string name;
    std::function<Core::Connection(EventBroker::EventBroker&)> subscribe;
    std::function<void(EventBroker::EventBroker&)> publish;
    std::function<void()> verify;
};

class EventBrokerContentTest : public ::testing::TestWithParam<ContentScenario>
{
   protected:
    EventBroker::EventBroker broker;
};

TEST_P(EventBrokerContentTest, ReceivedEventMatchesPublished)
{
    const auto& p = GetParam();
    auto conn = p.subscribe(broker);
    p.publish(broker);
    p.verify();
}

INSTANTIATE_TEST_SUITE_P(
    BrokerContentScenarios, EventBrokerContentTest,
    ::testing::Values(
        // Multi-field struct: all fields arrive intact
        []() {
            auto received = std::make_shared<MultiFieldEvent>();
            return ContentScenario{
                "MultiField_AllFieldsPreserved",
                [received](EventBroker::EventBroker& b) {
                    return b.subscribe<MultiFieldEvent>(
                        [received](const MultiFieldEvent& e) { *received = e; });
                },
                [](EventBroker::EventBroker& b) { b.publish(MultiFieldEvent{7, 3.14, "label"}); },
                [received]() {
                    EXPECT_EQ(received->id, 7);
                    EXPECT_DOUBLE_EQ(received->factor, 3.14);
                    EXPECT_EQ(received->label, "label");
                }};
        }(),

        // Negative integer: sign is not lost in the void* round-trip
        []() {
            auto received = std::make_shared<SimpleIntEvent>();
            return ContentScenario{
                "SimpleInt_NegativeValue",
                [received](EventBroker::EventBroker& b) {
                    return b.subscribe<SimpleIntEvent>(
                        [received](const SimpleIntEvent& e) { *received = e; });
                },
                [](EventBroker::EventBroker& b) { b.publish(SimpleIntEvent{-999}); },
                [received]() { EXPECT_EQ(received->value, -999); }};
        }(),

        // Special characters: heap string content preserved verbatim
        []() {
            auto received = std::make_shared<StringEvent>();
            const std::string special = "tab\there\nnewline\u00e9\u00e0";
            return ContentScenario{
                "String_SpecialCharacters",
                [received](EventBroker::EventBroker& b) {
                    return b.subscribe<StringEvent>(
                        [received](const StringEvent& e) { *received = e; });
                },
                [special](EventBroker::EventBroker& b) { b.publish(StringEvent{special}); },
                [received, special]() { EXPECT_EQ(received->message, special); }};
        }(),

        // Large payload: 1 000-byte buffer not truncated or corrupted
        []() {
            auto received = std::make_shared<LargePayloadEvent>();
            std::vector<uint8_t> payload(1000);
            for (size_t i = 0; i < payload.size(); ++i)
            {
                payload[i] = static_cast<uint8_t>(i % 256);
            }
            return ContentScenario{
                "LargePayload_DataIntact",
                [received](EventBroker::EventBroker& b) {
                    return b.subscribe<LargePayloadEvent>(
                        [received](const LargePayloadEvent& e) { *received = e; });
                },
                [payload](EventBroker::EventBroker& b) { b.publish(LargePayloadEvent{payload}); },
                [received, payload]() {
                    ASSERT_EQ(received->data.size(), payload.size());
                    EXPECT_EQ(received->data, payload);
                }};
        }(),

        // Nested struct: both inner and outer fields preserved
        []() {
            auto received = std::make_shared<NestedEvent>();
            return ContentScenario{
                "Nested_BothLevelsPreserved",
                [received](EventBroker::EventBroker& b) {
                    return b.subscribe<NestedEvent>(
                        [received](const NestedEvent& e) { *received = e; });
                },
                [](EventBroker::EventBroker& b) { b.publish(NestedEvent{{42}, "outer"}); },
                [received]() {
                    EXPECT_EQ(received->inner.value, 42);
                    EXPECT_EQ(received->tag, "outer");
                }};
        }()),
    [](const ::testing::TestParamInfo<ContentScenario>& info) { return info.param.name; });
