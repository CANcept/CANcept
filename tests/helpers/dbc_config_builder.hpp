#pragma once

#include <algorithm>
#include <list>
#include <string>
#include <utility>

#include "core/dto/dbc_dto.hpp"

namespace TestHelpers {

/**
 * @brief Fluent API builder for creating DBC signal descriptions.
 *
 * Example:
 * @code
 *   auto signal = DbcSignalBuilder("Speed")
 *       .startBit(0)
 *       .size(16)
 *       .littleEndian()
 *       .unsigned_()
 *       .factor(0.01)
 *       .offset(0)
 *       .range(0, 655.35)
 *       .unit("km/h")
 *       .build();
 * @endcode
 */
class DbcSignalBuilder
{
   public:
    explicit DbcSignalBuilder(std::string name) : m_signal{}
    {
        m_signal.signalName = std::move(name);
        m_signal.multiplexer = false;
        m_signal.multiplexedBy = -1;
        m_signal.startBit = 0;
        m_signal.signalSize = 8;
        m_signal.byteOrder = false;  // Little endian
        m_signal.valueType = false;  // Unsigned
        m_signal.factor = 1.0;
        m_signal.offset = 0.0;
        m_signal.minimum = 0.0;
        m_signal.maximum = 255.0;
        m_signal.unit = "";
    }

    DbcSignalBuilder& startBit(const uint bit)
    {
        m_signal.startBit = bit;
        return *this;
    }

    DbcSignalBuilder& size(const uint bits)
    {
        m_signal.signalSize = bits;
        if (!m_signal.valueType)
        {
            m_signal.maximum = (1ULL << bits) - 1;
        }
        return *this;
    }

    DbcSignalBuilder& littleEndian()
    {
        m_signal.byteOrder = false;
        return *this;
    }

    DbcSignalBuilder& bigEndian()
    {
        m_signal.byteOrder = true;
        return *this;
    }

    DbcSignalBuilder& unsigned_()
    {
        m_signal.valueType = false;
        return *this;
    }

    DbcSignalBuilder& signed_()
    {
        m_signal.valueType = true;
        return *this;
    }

    DbcSignalBuilder& factor(const double factor)
    {
        m_signal.factor = factor;
        return *this;
    }

    DbcSignalBuilder& offset(const double offset)
    {
        m_signal.offset = offset;
        return *this;
    }

    DbcSignalBuilder& range(const double min, const double max)
    {
        m_signal.minimum = min;
        m_signal.maximum = max;
        return *this;
    }

    DbcSignalBuilder& unit(std::string u)
    {
        m_signal.unit = std::move(u);
        return *this;
    }

    DbcSignalBuilder& receiver(std::string rcv)
    {
        m_signal.receivers.push_back(std::move(rcv));
        return *this;
    }

    DbcSignalBuilder& multiplexer(const bool isMux = true)
    {
        m_signal.multiplexer = isMux;
        return *this;
    }

    DbcSignalBuilder& multiplexedBy(const int value)
    {
        m_signal.multiplexedBy = value;
        return *this;
    }

    [[nodiscard]] Core::DbcSignalDescription build() const
    {
        return m_signal;
    }

    [[nodiscard]] explicit operator Core::DbcSignalDescription() const
    {
        return build();
    }

   private:
    Core::DbcSignalDescription m_signal;
};

/**
 * @brief Fluent API builder for creating DBC message descriptions.
 *
 * Example:
 * @code
 *   auto message = DbcMessageBuilder(0x123, "MotorStatus")
 *       .size(8)
 *       .transmitter("MotorController")
 *       .signal(DbcSignalBuilder("Speed").startBit(0).size(16).unit("rpm"))
 *       .signal(DbcSignalBuilder("Temperature").startBit(16).size(8).unit("C"))
 *       .build();
 * @endcode
 */
class DbcMessageBuilder
{
   public:
    DbcMessageBuilder(const uint id, std::string name) : m_message{}
    {
        m_message.messageId = id;
        m_message.messageName = std::move(name);
        m_message.messageSize = 8;
        m_message.transmitterName = "";
    }

    DbcMessageBuilder& size(const uint bytes)
    {
        m_message.messageSize = bytes;
        return *this;
    }

    DbcMessageBuilder& transmitter(std::string name)
    {
        m_message.transmitterName = std::move(name);
        return *this;
    }

    DbcMessageBuilder& signal(const Core::DbcSignalDescription& sig)
    {
        m_message.signalDescriptions.push_back(sig);
        return *this;
    }

    DbcMessageBuilder& signal(const DbcSignalBuilder& sigBuilder)
    {
        m_message.signalDescriptions.push_back(sigBuilder.build());
        return *this;
    }

    [[nodiscard]] Core::DbcMessageDescription build() const
    {
        return m_message;
    }

    [[nodiscard]] explicit operator Core::DbcMessageDescription() const
    {
        return build();
    }

   private:
    Core::DbcMessageDescription m_message;
};

/**
 * @brief Fluent API builder for creating complete DBC configurations.
 *
 * Example:
 * @code
 *   auto config = DbcConfigBuilder()
 *       .version("1.0")
 *       .fileName("test.dbc")
 *       .node("MotorController")
 *       .node("Sensor")
 *       .message(DbcMessageBuilder(0x100, "Speed")
 *           .signal(DbcSignalBuilder("Velocity").size(16).unit("m/s")))
 *       .build();
 * @endcode
 */
class DbcConfigBuilder
{
   public:
    DbcConfigBuilder() : m_config{}
    {
        m_config.metaData.version = "";
        m_config.metaData.fileName = "test.dbc";
    }

    DbcConfigBuilder& version(std::string ver)
    {
        m_config.metaData.version = std::move(ver);
        return *this;
    }

    DbcConfigBuilder& fileName(std::string name)
    {
        m_config.metaData.fileName = std::move(name);
        return *this;
    }

    DbcConfigBuilder& node(std::string nodeName)
    {
        m_config.nodeDefinitions.push_back(std::move(nodeName));
        return *this;
    }

    DbcConfigBuilder& message(const Core::DbcMessageDescription& msg)
    {
        m_config.messageDefinitions.push_back(msg);
        return *this;
    }

    DbcConfigBuilder& message(const DbcMessageBuilder& msgBuilder)
    {
        m_config.messageDefinitions.push_back(msgBuilder.build());
        return *this;
    }

    DbcConfigBuilder& comment(std::string cmt)
    {
        m_config.comments.push_back(std::move(cmt));
        return *this;
    }

    DbcConfigBuilder& signalValue(const uint msgId, std::string sigName, const double value,
                                  std::string meaning)
    {
        // Find existing signal value description or create new
        const auto it = std::ranges::find_if(
            m_config.signalValueDescriptions, [&](const Core::DbcSignalValueDescription& svd) {
                return svd.messageId == msgId && svd.signalName == sigName;
            });

        if (it != m_config.signalValueDescriptions.end())
        {
            it->signalDescriptions.push_back({value, std::move(meaning)});
        } else
        {
            Core::DbcSignalValueDescription signalValueDescription;
            signalValueDescription.messageId = msgId;
            signalValueDescription.signalName = std::move(sigName);
            signalValueDescription.signalDescriptions.push_back({value, std::move(meaning)});
            m_config.signalValueDescriptions.push_back(std::move(signalValueDescription));
        }
        return *this;
    }

    [[nodiscard]] Core::DbcConfig build() const
    {
        return m_config;
    }

    [[nodiscard]] explicit operator Core::DbcConfig() const
    {
        return build();
    }

   private:
    Core::DbcConfig m_config;
};

}  // namespace TestHelpers
