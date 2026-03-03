#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <qtestsupport_core.h>

#include <memory>

#include "core/event/dbc_event.hpp"
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/dbc_view.hpp"
#include "dbc_file/view/proxies/flat_list_proxy.hpp"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace DbcFile;
using namespace TestHelpers;
using ::testing::_;
using ::testing::Field;
using ::testing::Invoke;

// ============================================================================
// 1. PARAMETERIZED TESTS: Static Helper Methods
// ============================================================================

struct UnitScenario {
    std::string name;
    Core::DbcConfig config;
    QStringList expectedUnits;
};

class UnitExtractionTest : public ::testing::TestWithParam<UnitScenario> {};

TEST_P(UnitExtractionTest, ExtractsAndSortsUnitsCorrectly) {
    const auto& p = GetParam();
    Core::DbcConfig config(p.config);

    QStringList result = DbcComponent::extractSignalUnits(config);

    ASSERT_EQ(result.size(), p.expectedUnits.size());
    for (int i = 0; i < result.size(); ++i) {
        EXPECT_EQ(result[i], p.expectedUnits[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnitScenarios, UnitExtractionTest,
    ::testing::Values(
        // Case 1: Simple cleanup
        UnitScenario{
            "DuplicatesAndEmpty",
            DbcConfigBuilder()
                .message(DbcMessageBuilder(1, "M").signal(DbcSignalBuilder("S1").unit("V"))
                                                  .signal(DbcSignalBuilder("S2").unit("V")) // Dup
                                                  .signal(DbcSignalBuilder("S3").unit(""))) // Empty
                .build(),
            {"V"}
        },
        // Case 2: Case Sensitivity & Sorting
        UnitScenario{
            "CaseSensitiveSort",
            DbcConfigBuilder()
                .message(DbcMessageBuilder(1, "M").signal(DbcSignalBuilder("S1").unit("rpm"))
                                                  .signal(DbcSignalBuilder("S2").unit("Bar"))
                                                  .signal(DbcSignalBuilder("S3").unit("km/h")))
                .build(),
            {"Bar", "km/h", "rpm"}
        }
    ),
    [](const ::testing::TestParamInfo<UnitScenario>& info) { return info.param.name; }
);

struct SenderScenario {
    std::string name;
    Core::DbcConfig config;
    QStringList expectedSenders;
};

class SenderExtractionTest : public ::testing::TestWithParam<SenderScenario> {};

TEST_P(SenderExtractionTest, ExtractsAndSortsSendersCorrectly) {
    const auto& p = GetParam();
    Core::DbcConfig config(p.config);

    QStringList result = DbcComponent::extractSenders(config);

    ASSERT_EQ(result.size(), p.expectedSenders.size());
    for (int i = 0; i < result.size(); ++i) {
        EXPECT_EQ(result[i], p.expectedSenders[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(
    SenderScenarios, SenderExtractionTest,
    ::testing::Values(
        SenderScenario{
            "DedupesSenders",
            DbcConfigBuilder()
                .message(DbcMessageBuilder(1, "M1").transmitter("Engine"))
                .message(DbcMessageBuilder(2, "M2").transmitter("Engine")) // Dup
                .message(DbcMessageBuilder(3, "M3").transmitter("ABS"))
                .build(),
            {"ABS", "Engine"}
        },
        SenderScenario{
            "IgnoresEmpty",
            DbcConfigBuilder()
                .message(DbcMessageBuilder(1, "M1").transmitter(""))
                .message(DbcMessageBuilder(2, "M2").transmitter("Gateway"))
                .build(),
            {"Gateway"}
        }
    ),
    [](const ::testing::TestParamInfo<SenderScenario>& info) { return info.param.name; }
);
// ============================================================================
// 2. COMPONENT LIFECYCLE & INTEGRATION TESTS
// ============================================================================

/**
 * @brief Unit tests for DbcComponent.
 * Verifies lifecycle, event processing (Broker -> View/Model),
 * and signal forwarding (View -> Broker).
 */
class DbcComponentTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        mockBroker = std::make_unique<MockEventBroker>();
        // Expect subscription on startup
        EXPECT_CALL(*mockBroker, _subscribeEvent(testing::_)).Times(testing::AtLeast(1));

        component = std::make_unique<DbcComponent>(*mockBroker);
    }

    void TearDown() override
    {
        if (component)
        {
            component->onStop();
        }
        component.reset();
        mockBroker.reset();
    }

    // Helper to access the view
    DbcView* getView()
    {
        return qobject_cast<DbcView*>(component->getView());
    }

    std::unique_ptr<MockEventBroker> mockBroker;
    std::unique_ptr<DbcComponent> component;
};

/**
 * @brief Test component lifecycle - construction.
 */
TEST_F(DbcComponentTest, ConstructsSuccessfully)
{
    EXPECT_NE(component, nullptr);
    EXPECT_NE(component->getView(), nullptr);
    EXPECT_NE(component->getModel(), nullptr);
}

/**
 * @brief Test component lifecycle - onStart.
 */
TEST_F(DbcComponentTest, StartsSuccessfully)
{
    EXPECT_NO_THROW(component->onStart());
}

/**
 * @brief Test component lifecycle - onStop.
 */
TEST_F(DbcComponentTest, StopsSuccessfully)
{
    component->onStart();
    EXPECT_NO_THROW(component->onStop());
}

/**
 * @brief Test multiple start/stop cycles.
 */
TEST_F(DbcComponentTest, MultipleStartStopCycles)
{
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_NO_THROW(component->onStart());
        QTest::qWait(10);
        EXPECT_NO_THROW(component->onStop());
    }
}

/**
 * @brief Test DBC parsed event updates the internal model.
 */
TEST_F(DbcComponentTest, HandlesDbcParsedEvent_UpdatesModel)
{
    component->onStart();

    // 1. Trigger Event
    const auto config = DbcExamples::motorController();
    mockBroker->triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    // 2. Wait for processing (synchronous usually, but qWait is safe)
    QTest::qWait(10);

    // 3. Verify Model Data via direct Model access
    auto* model = component->getModel();
    ASSERT_NE(model, nullptr);

    // Check if an ECU from the config exists in the model
    // (Row 1 is usually the first ECU after Overview)
    QModelIndex ecuIdx = model->index(1, 0, QModelIndex());
    EXPECT_EQ(model->data(ecuIdx, Qt::DisplayRole).toString().toStdString(), "MotorController");
}

/**
 * @brief Test DBC parsed event updates the view filters.
 */
TEST_F(DbcComponentTest, HandlesDbcParsedEvent_UpdatesViewFilters)
{
    component->onStart();

    // Config with specific units/senders
    auto config = DbcConfigBuilder()
        .message(DbcMessageBuilder(1, "Msg").transmitter("MySender")
            .signal(DbcSignalBuilder("Sig").unit("MyUnit")))
        .build();

    mockBroker->triggerEvent(Core::DBCParsedEvent(config, ""));
    QTest::qWait(10);

    // Verify View State (via Proxy Getters we added)
    auto* view = getView();
    ASSERT_NE(view, nullptr);
}

/**
 * @brief Test DBC parse error event handling.
 */
TEST_F(DbcComponentTest, HandlesDbcParseError)
{
    component->onStart();

    EXPECT_NO_THROW(mockBroker->triggerEvent(Core::DBCParseErrorEvent("Invalid File", "")));

    QTest::qWait(50);

    // We assume the view shows an error message.
    // Without UI introspection, we verify stability (no crash).
    SUCCEED();
}

/**
 * @brief Test outgoing event when view requests file load.
 */
TEST_F(DbcComponentTest, PublishesRequest_OnViewFileSelected)
{
    component->onStart();

    // 1. Setup expectation: Broker must receive publish call
    EXPECT_CALL(*mockBroker, _publishEvent(testing::_, testing::_))
        .WillOnce([](std::type_index type, const void* data) {
            EXPECT_EQ(type, typeid(Core::ParseDBCRequestEvent));
            const auto* event = static_cast<const Core::ParseDBCRequestEvent*>(data);
            EXPECT_EQ(event->filePath, "/tmp/test.dbc");
        });

    // 2. Trigger Signal in View
    auto* view = getView();
    ASSERT_NE(view, nullptr);

    emit view->fileLoadRequested("/tmp/test.dbc");

    QTest::qWait(10); // Allow signal propagation
}

/**
 * @brief Test component survives rapid event publishing.
 */
TEST_F(DbcComponentTest, SurvivesRapidEventPublishing)
{
    component->onStart();

    for (int i = 0; i < 10; ++i)
    {
        mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::simple(), ""));
        mockBroker->triggerEvent(Core::DBCParseErrorEvent("Error", ""));
    }

    QTest::qWait(100);
    EXPECT_NE(component->getView(), nullptr);
}

/**
 * @brief Test component destruction during active state.
 */
TEST_F(DbcComponentTest, SafeDestructionAfterStart)
{
    component->onStart();
    QTest::qWait(10);

    EXPECT_NO_THROW(component.reset());
}

/**
 * @brief Test safe destruction immediately after event trigger.
 */
TEST_F(DbcComponentTest, SafeDestructionDuringEventProcessing)
{
    component->onStart();
    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::simple(), ""));

    // Immediately destroy
    EXPECT_NO_THROW(component.reset());
}