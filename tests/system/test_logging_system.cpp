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

#include <gtest/gtest.h>
#include <unistd.h>

#include <QDir>
#include <QFile>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <chrono>
#include <thread>

#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "event_broker/event_broker.hpp"
#include "logging/logging_component.hpp"
#include "tests/helpers/logging_system_helpers.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
#include "tests/helpers/temp_dbc_file.hpp"

class CanLoggingSystemTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        if (getuid() != 0)
        {
            GTEST_SKIP() << "System tests require root privileges (run start.sh with sudo)";
        }

        deviceManager = std::make_unique<TestHelpers::SocketCanDeviceManager>("vcan0");
        try
        {
            deviceManager->create();
            deviceManager->up();
            vcanCreated = true;
        } catch (const std::exception& e)
        {
            GTEST_SKIP() << "Failed to set up vcan0: " << e.what();
        }

        broker = std::make_unique<EventBroker::EventBroker>();
        canHandler = std::make_unique<CanHandler::CanCommunicationHandler>(*broker);
        dbcHandler = std::make_unique<CanHandler::DbcHandler>(*broker);
        logging = std::make_unique<Logging::LoggingComponent>(*broker);

        broker->publish<Core::AppStartedEvent>({});
        QTest::qWait(100);
    }

    void TearDown() override
    {
        if (broker)
        {
            broker->publish<Core::AppStoppedEvent>({});
            QTest::qWait(100);
        }

        logging.reset();
        dbcHandler.reset();
        canHandler.reset();
        broker.reset();

        if (vcanCreated)
        {
            deviceManager->down();
            deviceManager->remove();
        }
    }

    void connectVcan() const
    {
        broker->publish<Core::CanDriverChangeEvent>(Core::CanDriverChangeEvent{"vcan0"});
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    void loadDbc(const std::string& path) const
    {
        broker->publish<Core::ParseDBCRequestEvent>(Core::ParseDBCRequestEvent{path});
        QTest::qWait(100);
    }

    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    std::unique_ptr<EventBroker::EventBroker> broker;
    std::unique_ptr<CanHandler::CanCommunicationHandler> canHandler;
    std::unique_ptr<CanHandler::DbcHandler> dbcHandler;
    std::unique_ptr<Logging::LoggingComponent> logging;
    bool vcanCreated = false;
};

TEST_F(CanLoggingSystemTest, StartRawLogging_SendVcan_LogFileContainsMessage)
{
    connectVcan();

    const qint64 testStart = QDateTime::currentMSecsSinceEpoch();
    TestHelpers::acceptDialogAsRaw();
    auto* startBtn = logging->getView()->findChild<QPushButton*>();
    ASSERT_NE(startBtn, nullptr) << "Could not find start button in LoggingView";
    startBtn->click();
    QTest::qWait(500);

    Core::RawCanMessage msg;
    msg.messageId = 0x42;
    msg.dlc = 2;
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;
    broker->publish<Core::SendCanMessageRawEvent>(Core::SendCanMessageRawEvent{msg});

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    QMetaObject::invokeMethod(logging.get(), "stopLogging");
    QTest::qWait(300);

    const QString logFile = TestHelpers::findLogFileCreatedAfter(testStart);
    ASSERT_FALSE(logFile.isEmpty()) << "No log file was created during the session";

    QFile file(logFile);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());

    EXPECT_TRUE(content.contains("Timestamp,MessageId,Data")) << "Log file missing CSV header";
    const QStringList lines = content.trimmed().split('\n', Qt::SkipEmptyParts);
    EXPECT_GT(lines.size(), 1) << "Log file should contain at least one data row beyond the header";
    EXPECT_TRUE(content.contains("42")) << "Expected message ID 0x42 in log";
}

TEST_F(CanLoggingSystemTest, StartDbcLogging_SendVcan_LogFileContainsDecodedSignal)
{
    loadDbc(TestHelpers::makeTempDbcFile());
    connectVcan();

    const qint64 testStart = QDateTime::currentMSecsSinceEpoch();
    TestHelpers::acceptDialogAsDbc();
    auto* startBtn = logging->getView()->findChild<QPushButton*>();
    ASSERT_NE(startBtn, nullptr) << "Could not find start button in LoggingView";
    startBtn->click();
    QTest::qWait(500);

    Core::RawCanMessage msg;
    msg.messageId = TestHelpers::kTestMsgId;
    msg.dlc = 8;
    msg.data.fill(0);
    msg.data[0] = 0xAB;
    broker->publish<Core::SendCanMessageRawEvent>(Core::SendCanMessageRawEvent{msg});

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    QMetaObject::invokeMethod(logging.get(), "stopLogging");
    QTest::qWait(300);

    const QString logFile = TestHelpers::findLogFileCreatedAfter(testStart);
    ASSERT_FALSE(logFile.isEmpty()) << "No log file was created during the DBC session";

    QFile file(logFile);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());

    // Header must be: Timestamp,{MessageName}_{SignalName}_{Unit}
    const QString expectedHeader =
        QString("Timestamp,TestMessage_%1_units").arg(TestHelpers::kTestSignalName);
    EXPECT_TRUE(content.contains(expectedHeader))
        << "Log file header should be \"" << expectedHeader.toStdString() << "\"";

    EXPECT_TRUE(content.contains("171.000"))
        << "Log file should contain the decoded signal value 171.000";

    const QStringList lines = content.trimmed().split('\n', Qt::SkipEmptyParts);
    EXPECT_GT(lines.size(), 1) << "Log file should contain at least one decoded signal row";
}