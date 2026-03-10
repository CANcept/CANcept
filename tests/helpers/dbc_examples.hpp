#pragma once

#include "core/dto/dbc_dto.hpp"
#include "dbc_config_builder.hpp"

namespace TestHelpers {

/**
 * @brief Pre-built DBC configuration examples for quick testing.
 *
 * Provides common DBC configurations that can be used immediately in tests
 * without manually building them.
 *
 * Usage:
 * @code
 *   // Use a pre-built example
 *   auto config = DbcExamples::motorController();
 *
 *   // Publish to event broker
 *   eventBroker->publish(Core::DBCParsedEvent(config, "test.dbc"));
 * @endcode
 */
class DbcExamples
{
   public:
    /**
     * @brief Simple motor controller DBC.
     *
     * Messages:
     * - 0x100 MotorStatus: Speed  Temperature
     * - 0x101 MotorCommand: TargetSpeed (rpm), Enable
     *
     * Nodes: MotorController, Dashboard
     */
    [[nodiscard]] static Core::DbcConfig motorController()
    {
        return DbcConfigBuilder()
            .version("1.0")
            .fileName("motor_controller.dbc")
            .node("MotorController")
            .node("Dashboard")
            // Message 0x100: Motor Status
            .message(DbcMessageBuilder(0x100, "MotorStatus")
                         .size(8)
                         .transmitter("MotorController")
                         .signal(DbcSignalBuilder("Speed")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0)
                                     .range(0, 65535)
                                     .unit("rpm"))
                         .signal(DbcSignalBuilder("Temperature")
                                     .startBit(16)
                                     .size(8)
                                     .littleEndian()
                                     .signed_()
                                     .factor(1.0)
                                     .offset(-40)
                                     .range(-40, 215)
                                     .unit("C"))
                         .signal(DbcSignalBuilder("ErrorCode")
                                     .startBit(24)
                                     .size(8)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0)
                                     .range(0, 255)
                                     .unit("")))
            // Message 0x101: Motor Command
            .message(DbcMessageBuilder(0x101, "MotorCommand")
                         .size(8)
                         .transmitter("Dashboard")
                         .signal(DbcSignalBuilder("TargetSpeed")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0)
                                     .range(0, 65535)
                                     .unit("rpm"))
                         .signal(DbcSignalBuilder("Enable")
                                     .startBit(16)
                                     .size(1)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0)
                                     .range(0, 1)
                                     .unit("")))
            // Signal value descriptions
            .signalValue(0x100, "ErrorCode", 0, "No Error")
            .signalValue(0x100, "ErrorCode", 1, "Overheat")
            .signalValue(0x100, "ErrorCode", 2, "Overcurrent")
            .signalValue(0x101, "Enable", 0, "Disabled")
            .signalValue(0x101, "Enable", 1, "Enabled")
            .build();
    }

    /**
     * @brief Vehicle sensors DBC.
     *
     * Messages:
     * - 0x200 VehicleSpeed: Speed (km/h), Odometer (km)
     * - 0x201 Temperatures: EngineTemp (C), OilTemp (C), AmbientTemp (C)
     * - 0x202 BatteryStatus: Voltage (V), Current (A), StateOfCharge (%)
     *
     * Nodes: ECU, Sensor1, Sensor2
     */
    [[nodiscard]] static Core::DbcConfig vehicleSensors()
    {
        return DbcConfigBuilder()
            .version("1.0")
            .fileName("vehicle_sensors.dbc")
            .node("ECU")
            .node("Sensor1")
            .node("Sensor2")
            // Message 0x200: Vehicle Speed
            .message(DbcMessageBuilder(0x200, "VehicleSpeed")
                         .size(8)
                         .transmitter("ECU")
                         .signal(DbcSignalBuilder("Speed")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(0.01)
                                     .offset(0)
                                     .range(0, 655.35)
                                     .unit("km/h"))
                         .signal(DbcSignalBuilder("Odometer")
                                     .startBit(16)
                                     .size(32)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(0.1)
                                     .offset(0)
                                     .range(0, 429496729.5)
                                     .unit("km")))
            // Message 0x201: Temperatures
            .message(DbcMessageBuilder(0x201, "Temperatures")
                         .size(8)
                         .transmitter("Sensor1")
                         .signal(DbcSignalBuilder("EngineTemp")
                                     .startBit(0)
                                     .size(8)
                                     .littleEndian()
                                     .signed_()
                                     .factor(1.0)
                                     .offset(-40)
                                     .range(-40, 215)
                                     .unit("C"))
                         .signal(DbcSignalBuilder("OilTemp")
                                     .startBit(8)
                                     .size(8)
                                     .littleEndian()
                                     .signed_()
                                     .factor(1.0)
                                     .offset(-40)
                                     .range(-40, 215)
                                     .unit("C"))
                         .signal(DbcSignalBuilder("AmbientTemp")
                                     .startBit(16)
                                     .size(8)
                                     .littleEndian()
                                     .signed_()
                                     .factor(1.0)
                                     .offset(-40)
                                     .range(-40, 215)
                                     .unit("C")))
            // Message 0x202: Battery Status
            .message(DbcMessageBuilder(0x202, "BatteryStatus")
                         .size(8)
                         .transmitter("Sensor2")
                         .signal(DbcSignalBuilder("Voltage")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(0.01)
                                     .offset(0)
                                     .range(0, 655.35)
                                     .unit("V"))
                         .signal(DbcSignalBuilder("Current")
                                     .startBit(16)
                                     .size(16)
                                     .littleEndian()
                                     .signed_()
                                     .factor(0.1)
                                     .offset(0)
                                     .range(-3276.8, 3276.7)
                                     .unit("A"))
                         .signal(DbcSignalBuilder("StateOfCharge")
                                     .startBit(32)
                                     .size(8)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0)
                                     .range(0, 100)
                                     .unit("%")))
            .build();
    }

    /**
     * @brief Simple test DBC with one message and one signal.
     *
     * Messages:
     * - 0x123 TestMessage: TestSignal (0-255)
     *
     * Nodes: TestNode
     */
    [[nodiscard]] static Core::DbcConfig simple()
    {
        return DbcConfigBuilder()
            .version("1.0")
            .fileName("simple.dbc")
            .node("TestNode")
            .message(DbcMessageBuilder(0x123, "TestMessage")
                         .size(8)
                         .transmitter("TestNode")
                         .signal(DbcSignalBuilder("TestSignal")
                                     .startBit(0)
                                     .size(8)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0)
                                     .range(0, 255)
                                     .unit("")))
            .build();
    }

    /**
     * @brief DBC with multiple signals in one message.
     *
     * Messages:
     * - 0x300 MultiSignal: Signal1, Signal2, Signal3, Signal4, Signal5
     *
     * Useful for testing signal parsing with different bit positions.
     */
    [[nodiscard]] static Core::DbcConfig multiSignal()
    {
        return DbcConfigBuilder()
            .version("1.0")
            .fileName("multi_signal.dbc")
            .node("TestNode")
            .message(
                DbcMessageBuilder(0x300, "MultiSignal")
                    .size(8)
                    .transmitter("TestNode")
                    .signal(
                        DbcSignalBuilder("Signal1").startBit(0).size(8).littleEndian().unsigned_())
                    .signal(
                        DbcSignalBuilder("Signal2").startBit(8).size(8).littleEndian().unsigned_())
                    .signal(DbcSignalBuilder("Signal3")
                                .startBit(16)
                                .size(16)
                                .littleEndian()
                                .unsigned_())
                    .signal(
                        DbcSignalBuilder("Signal4").startBit(32).size(16).littleEndian().signed_())
                    .signal(DbcSignalBuilder("Signal5")
                                .startBit(48)
                                .size(8)
                                .littleEndian()
                                .unsigned_()))
            .build();
    }

    /**
     * @brief DBC with signals using different byte orders.
     *
     * Messages:
     * - 0x400 ByteOrderTest: LittleEndianSignal, BigEndianSignal
     *
     * Useful for testing byte order handling.
     */
    [[nodiscard]] static Core::DbcConfig byteOrderTest()
    {
        return DbcConfigBuilder()
            .version("1.0")
            .fileName("byte_order.dbc")
            .node("TestNode")
            .message(DbcMessageBuilder(0x400, "ByteOrderTest")
                         .size(8)
                         .transmitter("TestNode")
                         .signal(DbcSignalBuilder("LittleEndianSignal")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0))
                         .signal(DbcSignalBuilder("BigEndianSignal")
                                     .startBit(16)
                                     .size(16)
                                     .bigEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0)))
            .build();
    }

    /**
     * @brief DBC with signals using scaling (factor and offset).
     *
     * Messages:
     * - 0x500 ScalingTest: ScaledSignal (physical = raw * 0.1 + 10)
     *
     * Useful for testing signal value conversion.
     */
    [[nodiscard]] static Core::DbcConfig scalingTest()
    {
        return DbcConfigBuilder()
            .version("1.0")
            .fileName("scaling.dbc")
            .node("TestNode")
            .message(DbcMessageBuilder(0x500, "ScalingTest")
                         .size(8)
                         .transmitter("TestNode")
                         .signal(DbcSignalBuilder("ScaledSignal")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(0.1)
                                     .offset(10.0)
                                     .range(10.0, 6563.5)
                                     .unit("units")))
            .build();
    }

    /**
     * @brief Empty DBC configuration.
     *
     * No messages, no nodes. Useful for testing empty/invalid configurations.
     */
    [[nodiscard]] static Core::DbcConfig empty()
    {
        return DbcConfigBuilder().version("1.0").fileName("empty.dbc").build();
    }

    /**
     * @brief Large-scale DBC for stress/performance and parser coverage tests.
     *
     * Creates 120 messages (0x600-0x677), each with two signals:
     * - ValueN: 16-bit unsigned, little endian
     * - StatusN: 8-bit unsigned, little endian
     */
    [[nodiscard]] static auto longDbc() -> Core::DbcConfig
    {
        DbcConfigBuilder builder;
        builder.version("1.0").fileName("large_scale_120_messages.dbc").node("LoadTestNode");

        for (uint i = 0; i < 120; ++i)
        {
            const auto id = 0x000 + i;
            const auto suffix = std::to_string(i);

            builder.message(DbcMessageBuilder(id, "LargeMessage" + suffix)
                                .size(8)
                                .transmitter("LoadTestNode")
                                .signal(DbcSignalBuilder("Value" + suffix)
                                            .startBit(0)
                                            .size(16)
                                            .littleEndian()
                                            .unsigned_()
                                            .factor(1.0)
                                            .offset(0)
                                            .range(0, 65535)
                                            .unit("raw"))
                                .signal(DbcSignalBuilder("Status" + suffix)
                                            .startBit(16)
                                            .size(8)
                                            .littleEndian()
                                            .unsigned_()
                                            .factor(1.0)
                                            .offset(0)
                                            .range(0, 255)
                                            .unit("")));
        }

        return builder.build();
    }

    /**
     * @brief DBC with a single message containing 10 signals.
     *
     * Messages:
     * - 0x800 ComplexMessage: Signal0-Signal9 (8-bit each, little endian)
     *
     * Useful for testing dense signal packing in a single message.
     */
    [[nodiscard]] static auto manySignalMessage() -> Core::DbcConfig
    {
        auto builder = DbcConfigBuilder()
                           .version("1.0")
                           .fileName("multi_signal_message.dbc")
                           .node("ComplexNode");

        auto messageBuilder =
            DbcMessageBuilder(0x100, "ComplexMessage").size(8).transmitter("ComplexNode");

        for (uint i = 0; i < 20; ++i)
        {
            const auto startBit = i * 8;
            // Stop at 64 bits (8 bytes) to not exceed message size
            if (startBit >= 64) break;

            messageBuilder.signal(DbcSignalBuilder("Signal" + std::to_string(i))
                                      .startBit(startBit)
                                      .size(1)
                                      .littleEndian()
                                      .unsigned_()
                                      .factor(1.0)
                                      .offset(0)
                                      .range(0, 1)
                                      .unit(""));
        }

        builder.message(messageBuilder);
        return builder.build();
    }
};

}  // namespace TestHelpers
