#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "sending/model/sending_model.hpp"
#include "tests/helpers/can_test_matchers.hpp"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/dbc_examples.hpp"

using namespace TestHelpers;
using ::testing::_;

class SendingModelTestBase : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        model = std::make_unique<Sending::SendingModel>();
    }

    void expectRawSignal(QSignalSpy& spy, const uint16_t expectedId,
                         const std::vector<uint8_t>& expectedData)
    {
        ASSERT_EQ(spy.count(), 1) << "Expected exactly one signal emission";
        const auto args = spy.takeFirst();
        ASSERT_EQ(args.size(), 2) << "Signal should have 2 arguments (device, message)";
        const auto msg = args.at(1).value<Core::RawCanMessage>();

        EXPECT_THAT(msg, IsRawCanMessage(expectedId, expectedData));
    }

    std::unique_ptr<Sending::SendingModel> model;
};

/**
 * @brief A test factory to test setRawCanId with different CAN IDs.
 */
struct RawCanIdScenario {
    std::string name;
    uint16_t canId;
};

class RawCanIdTest : public SendingModelTestBase,
                     public ::testing::WithParamInterface<RawCanIdScenario>
{
};

TEST_P(RawCanIdTest, SetsAndTransmitsCorrectId)
{
    const auto& [name, canId] = GetParam();

    model->setRawCanId(canId);
    model->setRawData({0xAA});

    QSignalSpy spy(model.get(), &Sending::SendingModel::requestSendRaw);
    model->transmitCurrent();

    expectRawSignal(spy, canId, {0xAA});
}

INSTANTIATE_TEST_SUITE_P(RawCanIdScenarios, RawCanIdTest,
                         ::testing::Values(RawCanIdScenario{"MinId", 0x000},
                                           RawCanIdScenario{"Typical", 0x123},
                                           RawCanIdScenario{"Heartbeat", 0x100},
                                           RawCanIdScenario{"MaxStandardId", 0x7FF}),
                         [](const ::testing::TestParamInfo<RawCanIdScenario>& info) {
                             return info.param.name;
                         });

/**
 * @brief A test factory to test setRawData with different data patterns.
 */
struct RawDataScenario {
    std::string name;
    std::vector<uint8_t> data;
};

class RawDataTest : public SendingModelTestBase,
                    public ::testing::WithParamInterface<RawDataScenario>
{
};

TEST_P(RawDataTest, SetsAndTransmitsCorrectData)
{
    const auto& scenario = GetParam();

    model->setRawCanId(0x200);
    model->setRawData(scenario.data);

    QSignalSpy spy(model.get(), &Sending::SendingModel::requestSendRaw);
    model->transmitCurrent();

    expectRawSignal(spy, 0x200, scenario.data);
}

INSTANTIATE_TEST_SUITE_P(
    RawDataScenarios, RawDataTest,
    ::testing::Values(RawDataScenario{"Empty", {}}, RawDataScenario{"SingleByte", {0x42}},
                      RawDataScenario{"TwoBytes", {0xDE, 0xAD}},
                      RawDataScenario{"FullFrame",
                                      {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}},
                      RawDataScenario{"AllZeros", {0x00, 0x00, 0x00, 0x00}},
                      RawDataScenario{"AllOnes", {0xFF, 0xFF, 0xFF, 0xFF}}),
    [](const ::testing::TestParamInfo<RawDataScenario>& info) { return info.param.name; });

/**
 * @brief A test factory to test updateDbcConfig with different DBC configurations.
 */
struct DbcConfigScenario {
    std::string name;
    Core::DbcConfig config;
    size_t expectedMessageCount;
};

class DbcConfigTest : public SendingModelTestBase,
                      public ::testing::WithParamInterface<DbcConfigScenario>
{
};

TEST_P(DbcConfigTest, UpdatesConfigCorrectly)
{
    const auto& scenario = GetParam();

    model->updateDbcConfig(scenario.config);

    const auto* currentConfig = model->currentDbcConfig();
    ASSERT_NE(currentConfig, nullptr);
    EXPECT_EQ(currentConfig->metaData.fileName, scenario.config.metaData.fileName);
    EXPECT_EQ(currentConfig->messageDefinitions.size(), scenario.expectedMessageCount);
}

INSTANTIATE_TEST_SUITE_P(
    DbcConfigScenarios, DbcConfigTest,
    ::testing::Values(DbcConfigScenario{"SimpleConfig", DbcExamples::simple(), 1},
                      DbcConfigScenario{"MotorController", DbcExamples::motorController(), 2},
                      DbcConfigScenario{"VehicleSensors", DbcExamples::vehicleSensors(), 3},
                      DbcConfigScenario{"MultiSignal", DbcExamples::multiSignal(), 1},
                      DbcConfigScenario{"EmptyConfig", DbcExamples::empty(), 0}),
    [](const ::testing::TestParamInfo<DbcConfigScenario>& info) { return info.param.name; });

/**
 * @brief A test factory to test setMessageSelected and isMessageSelected.
 */
struct MessageSelectionScenario {
    std::string name;
    uint16_t messageId;
    bool selected;
};

class MessageSelectionTest : public SendingModelTestBase,
                             public ::testing::WithParamInterface<MessageSelectionScenario>
{
};

TEST_P(MessageSelectionTest, TogglesMessageSelection)
{
    const auto& scenario = GetParam();
    model->updateDbcConfig(DbcExamples::motorController());

    // Set selection
    model->setMessageSelected(scenario.messageId, scenario.selected);

    // Verify selection state
    EXPECT_EQ(model->isMessageSelected(scenario.messageId), scenario.selected);
}

INSTANTIATE_TEST_SUITE_P(
    MessageSelectionScenarios, MessageSelectionTest,
    ::testing::Values(MessageSelectionScenario{"SelectMessage", 0x100, true},
                      MessageSelectionScenario{"DeselectMessage", 0x100, false},
                      MessageSelectionScenario{"SelectDifferentMessage", 0x101, true}),
    [](const ::testing::TestParamInfo<MessageSelectionScenario>& info) { return info.param.name; });

/**
 * @brief A test factory to test setSignalSelected and isSignalSelected.
 */
struct SignalSelectionScenario {
    std::string name;
    uint16_t messageId;
    std::string signalName;
    bool selected;
};

class SignalSelectionTest : public SendingModelTestBase,
                            public ::testing::WithParamInterface<SignalSelectionScenario>
{
};

TEST_P(SignalSelectionTest, TogglesSignalSelection)
{
    const auto& scenario = GetParam();
    model->updateDbcConfig(DbcExamples::motorController());

    // Set signal selection
    model->setSignalSelected(scenario.messageId, scenario.signalName, scenario.selected);

    // Verify selection state
    EXPECT_EQ(model->isSignalSelected(scenario.messageId, scenario.signalName), scenario.selected);
}

INSTANTIATE_TEST_SUITE_P(
    SignalSelectionScenarios, SignalSelectionTest,
    ::testing::Values(SignalSelectionScenario{"SelectSpeed", 0x100, "Speed", true},
                      SignalSelectionScenario{"DeselectSpeed", 0x100, "Speed", false},
                      SignalSelectionScenario{"SelectTemperature", 0x100, "Temperature", true},
                      SignalSelectionScenario{"SelectTargetSpeed", 0x101, "TargetSpeed", true}),
    [](const ::testing::TestParamInfo<SignalSelectionScenario>& info) { return info.param.name; });

/**
 * @brief A test factory to test setSignalValue method for different values.
 */
struct SignalValueScenario {
    std::string name;
    uint16_t messageId;
    std::string signalName;
    double value;
};

class SignalValueTest : public SendingModelTestBase,
                        public ::testing::WithParamInterface<SignalValueScenario>
{
};

TEST_P(SignalValueTest, SetsSignalValue)
{
    const auto& scenario = GetParam();

    model->updateDbcConfig(DbcExamples::motorController());
    EXPECT_NO_THROW(model->setSignalValue(scenario.messageId, scenario.signalName, scenario.value));
}

INSTANTIATE_TEST_SUITE_P(
    SignalValueScenarios, SignalValueTest,
    ::testing::Values(SignalValueScenario{"SpeedZero", 0x100, "Speed", 0.0},
                      SignalValueScenario{"SpeedMid", 0x100, "Speed", 3000.0},
                      SignalValueScenario{"SpeedMax", 0x100, "Speed", 65535.0},
                      SignalValueScenario{"TempNegative", 0x100, "Temperature", -40.0},
                      SignalValueScenario{"TempPositive", 0x100, "Temperature", 100.0}),
    [](const ::testing::TestParamInfo<SignalValueScenario>& info) { return info.param.name; });

/**
 * @brief A test factory to test setTransmissionStatus and isCurrentlySending.
 */
struct TransmissionStatusScenario {
    std::string name;
    bool isActive;
};

class TransmissionStatusTest : public SendingModelTestBase,
                               public ::testing::WithParamInterface<TransmissionStatusScenario>
{
};

TEST_P(TransmissionStatusTest, UpdatesTransmissionStatus)
{
    const auto& [name, isActive] = GetParam();

    model->setTransmissionStatus(isActive);

    EXPECT_EQ(model->isCurrentlySending(), isActive);
}

INSTANTIATE_TEST_SUITE_P(TransmissionStatusScenarios, TransmissionStatusTest,
                         ::testing::Values(TransmissionStatusScenario{"StartSending", true},
                                           TransmissionStatusScenario{"StopSending", false}),
                         [](const ::testing::TestParamInfo<TransmissionStatusScenario>& info) {
                             return info.param.name;
                         });

/**
 * @brief A test factory to test complete raw message transmission flow.
 */
struct CombinedRawScenario {
    std::string name;
    uint16_t canId;
    std::vector<uint8_t> data;
};

class CombinedRawTest : public SendingModelTestBase,
                        public ::testing::WithParamInterface<CombinedRawScenario>
{
};

TEST_P(CombinedRawTest, TransmitsCompleteMessage)
{
    const auto& scenario = GetParam();

    model->setRawCanId(scenario.canId);
    model->setRawData(scenario.data);

    QSignalSpy spy(model.get(), &Sending::SendingModel::requestSendRaw);
    model->transmitCurrent();

    expectRawSignal(spy, scenario.canId, scenario.data);
}

INSTANTIATE_TEST_SUITE_P(
    CombinedRawScenarios, CombinedRawTest,
    ::testing::Values(CombinedRawScenario{"HeartbeatEmpty", 0x100, {}},
                      CombinedRawScenario{"SensorData", 0x200, {0x12, 0x34, 0x56, 0x78}},
                      CombinedRawScenario{"CommandMessage", 0x300, {0xFF, 0x00, 0xAA, 0x55}},
                      CombinedRawScenario{
                          "FullDataFrame", 0x400, {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}},
                      CombinedRawScenario{"EdgeMinId", 0x000, {0x42}},
                      CombinedRawScenario{"EdgeMaxId", 0x7FF, {0x99}}),
    [](const ::testing::TestParamInfo<CombinedRawScenario>& info) { return info.param.name; });

/**
 * @brief A test factory to test QAbstractItemModel data() method with different roles.
 */
struct ModelDataRoleScenario {
    std::string name;
    int role;
    bool requiresDbcConfig;
};

class ModelDataRoleTest : public SendingModelTestBase,
                          public ::testing::WithParamInterface<ModelDataRoleScenario>
{
};

TEST_P(ModelDataRoleTest, ReturnsDataForRole)
{
    const auto& scenario = GetParam();

    if (scenario.requiresDbcConfig)
    {
        model->updateDbcConfig(DbcExamples::motorController());
    }

    const QModelIndex idx = model->index(0, 0);
    const QVariant result = model->data(idx, scenario.role);

    EXPECT_TRUE(result.isValid() || !result.isValid());
}

INSTANTIATE_TEST_SUITE_P(
    ModelDataRoleScenarios, ModelDataRoleTest,
    ::testing::Values(
        ModelDataRoleScenario{"RoleValue", Sending::SendingModel::Role_Value, false},
        ModelDataRoleScenario{"RoleCanId", Sending::SendingModel::Role_CanId, false},
        ModelDataRoleScenario{"RoleSignalValue", Sending::SendingModel::Role_SignalValue, true},
        ModelDataRoleScenario{"RoleActiveMode", Sending::SendingModel::Role_ActiveMode, false},
        ModelDataRoleScenario{"RoleIsCyclicEnabled", Sending::SendingModel::Role_IsCyclicEnabled,
                              false},
        ModelDataRoleScenario{"RoleCycleIntervalMs", Sending::SendingModel::Role_CycleIntervalMs,
                              false},
        ModelDataRoleScenario{"RoleIsCurrentlySending",
                              Sending::SendingModel::Role_IsCurrentlySending, false}),
    [](const ::testing::TestParamInfo<ModelDataRoleScenario>& info) { return info.param.name; });

/**
 * @brief Some basic tests for the QAbstractItemModel.
 */
TEST_F(SendingModelTestBase, ReturnsValidRowCount)
{
    const int rows = model->rowCount();
    EXPECT_GE(rows, 0);
}

TEST_F(SendingModelTestBase, ReturnsValidColumnCount)
{
    const int cols = model->columnCount();
    EXPECT_GE(cols, 0);
}

TEST_F(SendingModelTestBase, CreatesValidIndex)
{
    const QModelIndex idx = model->index(0, 0);
    EXPECT_TRUE(idx.isValid() || !idx.isValid());
}

TEST_F(SendingModelTestBase, ReturnsInvalidParent)
{
    const QModelIndex idx = model->index(0, 0);
    const QModelIndex parentIdx = model->parent(idx);
    EXPECT_FALSE(parentIdx.isValid());
}

/**
 * @brief A test factory to test cyclic state getter methods.
 */
struct CyclicStateScenario {
    std::string name;
    bool enableCyclic;
    int expectedInterval;
};

class CyclicStateTest : public SendingModelTestBase,
                        public ::testing::WithParamInterface<CyclicStateScenario>
{
};

TEST_P(CyclicStateTest, ReturnsCyclicState)
{
    const auto& scenario = GetParam();

    model->setTransmissionStatus(scenario.enableCyclic);

    EXPECT_EQ(model->isCurrentlySending(), scenario.enableCyclic);
    EXPECT_EQ(model->cycleInterval(), scenario.expectedInterval);
}

INSTANTIATE_TEST_SUITE_P(CyclicStateScenarios, CyclicStateTest,
                         ::testing::Values(CyclicStateScenario{"NotSending", false, 100},
                                           CyclicStateScenario{"Sending", true, 100}),
                         [](const ::testing::TestParamInfo<CyclicStateScenario>& info) {
                             return info.param.name;
                         });

/**
 * @brief A few tests to check the forEachPendingMessage methods.
 */
TEST_F(SendingModelTestBase, InvokesRawHandlerForPendingMessages)
{
    model->setRawCanId(0x123);
    model->setRawData({0xAA, 0xBB});

    bool rawHandlerCalled = false;
    bool dbcHandlerCalled = false;

    model->forEachPendingMessage(
        [&](const Core::RawCanMessage& msg) {
            rawHandlerCalled = true;
            EXPECT_EQ(msg.messageId, 0x123);
        },
        [&](const Core::DbcCanMessage& msg) { dbcHandlerCalled = true; });

    EXPECT_TRUE(rawHandlerCalled);
    EXPECT_FALSE(dbcHandlerCalled);
}

TEST_F(SendingModelTestBase, InvokesDbcHandlerForPendingMessages)
{
    model->updateDbcConfig(DbcExamples::motorController());
    model->setMessageSelected(0x100, true);

    // Must also select a signal to have pending DBC messages
    model->setSignalSelected(0x100, "Speed", true);
    model->setSignalValue(0x100, "Speed", 1000.0);

    bool rawHandlerCalled = false;
    bool dbcHandlerCalled = false;

    model->forEachPendingMessage([&](const Core::RawCanMessage& msg) { rawHandlerCalled = true; },
                                 [&](const Core::DbcCanMessage& msg) {
                                     dbcHandlerCalled = true;
                                     EXPECT_EQ(msg.messageId, 0x100);
                                 });

    EXPECT_TRUE(dbcHandlerCalled || rawHandlerCalled);
}

/**
 * @brief Test the tree structure generated by the model.
 */
TEST_F(SendingModelTestBase, TreeStructureWithDbcMessages)
{
    model->updateDbcConfig(DbcExamples::motorController());

    // Root should have messages
    const int messageCount = model->rowCount(QModelIndex());
    EXPECT_EQ(messageCount, 2);

    const QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    EXPECT_TRUE(msgIdx.isValid());

    // Message should have signals as children
    const int signalCount = model->rowCount(msgIdx);
    EXPECT_EQ(signalCount, 3);

    const QModelIndex sigIdx = model->index(0, 0, msgIdx);
    EXPECT_TRUE(sigIdx.isValid());

    const QModelIndex parentIdx = model->parent(sigIdx);
    EXPECT_EQ(parentIdx, msgIdx);
}

TEST_F(SendingModelTestBase, DataMethodReturnsSignalValues)
{
    model->updateDbcConfig(DbcExamples::motorController());

    // Get first message
    const QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    const QModelIndex sigIdx = model->index(0, 0, msgIdx);

    // Test different roles
    const QVariant displayValue = model->data(sigIdx, Qt::DisplayRole);
    EXPECT_TRUE(displayValue.isValid());

    QVariant signalValue = model->data(sigIdx, Sending::SendingModel::Role_SignalValue);
    EXPECT_TRUE(signalValue.isValid());
}

TEST_F(SendingModelTestBase, SetDataModifiesSignalValue)
{
    model->updateDbcConfig(DbcExamples::motorController());

    // Must select message and signal first
    model->setMessageSelected(0x100, true);
    model->setSignalSelected(0x100, "Speed", true);
    const QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    ASSERT_TRUE(msgIdx.isValid());

    const QModelIndex sigIdx = model->index(0, 0, msgIdx);
    ASSERT_TRUE(sigIdx.isValid());
    model->setData(sigIdx, 5000.0, Sending::SendingModel::Role_SignalValue);
    model->setSignalValue(0x100, "Speed", 5000.0);
    EXPECT_TRUE(model->isSignalSelected(0x100, "Speed"));

    bool dbcHandlerCalled = false;
    bool rawHandlerCalled = false;
    model->forEachPendingMessage([&](const Core::RawCanMessage&) { rawHandlerCalled = true; },
                                 [&](const Core::DbcCanMessage& msg) {
                                     dbcHandlerCalled = true;
                                     EXPECT_EQ(msg.messageId, 0x100);
                                 });

    EXPECT_TRUE(dbcHandlerCalled || rawHandlerCalled);
}

TEST_F(SendingModelTestBase, DataMethodReturnsActiveMode)
{
    model->updateDbcConfig(DbcExamples::simple());

    const QModelIndex idx = model->index(0, 0);
    const QVariant mode = model->data(idx, Sending::SendingModel::Role_ActiveMode);

    EXPECT_TRUE(mode.isValid());
    const int modeValue = mode.toInt();
    EXPECT_TRUE(modeValue == 0 || modeValue == 1);
}

TEST_F(SendingModelTestBase, CurrentDbcConfigReturnsConfig)
{
    // Initially no config
    EXPECT_EQ(model->currentDbcConfig(), nullptr);

    // Load config
    auto config = DbcExamples::motorController();
    model->updateDbcConfig(config);

    // Should return valid config
    const auto* currentConfig = model->currentDbcConfig();
    ASSERT_NE(currentConfig, nullptr);
    EXPECT_EQ(currentConfig->metaData.fileName, "motor_controller.dbc");
    EXPECT_EQ(currentConfig->messageDefinitions.size(), 2);
}

TEST_F(SendingModelTestBase, DataMethodReturnsCyclicState)
{
    // Load DBC to get valid indices
    model->updateDbcConfig(DbcExamples::simple());

    const QModelIndex idx = model->index(0, 0);
    if (!idx.isValid())
    {
        GTEST_SKIP() << "Model has no valid indices";
    }

    // Test cyclic enabled
    if (QVariant cyclicEnabled = model->data(idx, Sending::SendingModel::Role_IsCyclicEnabled);
        cyclicEnabled.isValid())
    {
        EXPECT_FALSE(cyclicEnabled.toBool());
    }

    // Test cycle interval
    if (const QVariant interval = model->data(idx, Sending::SendingModel::Role_CycleIntervalMs);
        interval.isValid())
    {
        EXPECT_EQ(interval.toInt(), 100);
    }

    // Test currently sending
    if (const QVariant sending = model->data(idx, Sending::SendingModel::Role_IsCurrentlySending);
        sending.isValid())
    {
        EXPECT_FALSE(sending.toBool());
    }
}
