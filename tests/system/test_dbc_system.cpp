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

#include <QLabel>
#include <QStandardItemModel>
#include <QTest>
#include <atomic>
#include <string>

#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "core/event/dbc_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/dbc_view.hpp"
#include "dbc_file/view/pages/load_page.hpp"
#include "event_broker/event_broker.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
#include "tests/helpers/temp_dbc_file.hpp"

class DbcSystemTest : public ::testing::Test
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
        dbcComponent = std::make_unique<DbcFile::DbcComponent>(*broker);

        broker->publish<Core::AppStartedEvent>({});
        QTest::qWait(100);
    }

    void TearDown() override
    {
        if (broker)
        {
            broker->publish<Core::AppStoppedEvent>({});
            QTest::qWait(50);
        }

        dbcComponent.reset();
        dbcHandler.reset();
        canHandler.reset();
        broker.reset();

        if (vcanCreated)
        {
            deviceManager->down();
            deviceManager->remove();
        }
    }

    void loadDbc(const std::string& path) const
    {
        broker->publish<Core::ParseDBCRequestEvent>(Core::ParseDBCRequestEvent{path});
        QTest::qWait(2000);
    }

    auto getDbcView() const -> DbcFile::DbcView*
    {
        return qobject_cast<DbcFile::DbcView*>(dbcComponent->getView());
    }

    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    std::unique_ptr<EventBroker::EventBroker> broker;
    std::unique_ptr<CanHandler::CanCommunicationHandler> canHandler;
    std::unique_ptr<CanHandler::DbcHandler> dbcHandler;
    std::unique_ptr<DbcFile::DbcComponent> dbcComponent;
    bool vcanCreated = false;
};

TEST_F(DbcSystemTest, LoadDbc_ShowsSuccessLabel_AndParsesCorrectMessage)
{
    std::atomic<bool> parsedEventReceived{false};
    auto parsedConn = broker->subscribe<Core::DBCParsedEvent>(
        [&](const Core::DBCParsedEvent&) { parsedEventReceived = true; });
    loadDbc(TestHelpers::makeTempDbcFile());

    ASSERT_TRUE(parsedEventReceived.load()) << "DBCParsedEvent was never published";

    auto* dbcView = getDbcView();
    ASSERT_NE(dbcView, nullptr) << "DbcView could not be cast from component view";

    const auto& loadPage = dbcView->getLoadPage();
    QLabel* statusLabel = nullptr;
    for (auto* label : loadPage.findChildren<QLabel*>())
    {
        if (label->text() == DbcFile::Constants::Status::ParseSuccess)
        {
            statusLabel = label;
            break;
        }
    }
    ASSERT_NE(statusLabel, nullptr)
        << "No QLabel with text \"" << DbcFile::Constants::Status::ParseSuccess.toStdString()
        << "\" found inside LoadPage";
    EXPECT_FALSE(statusLabel->isHidden()) << "Status label should not be hidden after parse";

    auto* sidebar = dbcView->findChild<Core::Sidebar*>();
    ASSERT_NE(sidebar, nullptr) << "Sidebar not found inside DbcView";

    auto* sidebarModel = qobject_cast<QStandardItemModel*>(sidebar->model());
    ASSERT_NE(sidebarModel, nullptr) << "Sidebar model is not a QStandardItemModel";
    ASSERT_GE(sidebarModel->rowCount(), 2) << "Sidebar should have at least 2 tabs after DBC load";

    for (int i = 1; i < sidebarModel->rowCount(); ++i)
    {
        auto* item = sidebarModel->item(i);
        ASSERT_NE(item, nullptr);
        EXPECT_TRUE(item->isEnabled())
            << "Sidebar tab at index " << i << " should be enabled after DBC load";
    }
}

TEST_F(DbcSystemTest, LoadDbc_ParsedSignal_HasCorrectAttributes)
{
    Core::DbcConfig capturedConfig;
    std::atomic<bool> parsedEventReceived{false};

    auto parsedConn =
        broker->subscribe<Core::DBCParsedEvent>([&](const Core::DBCParsedEvent& event) {
            capturedConfig = event.config;
            parsedEventReceived = true;
        });

    loadDbc(TestHelpers::makeTempDbcFile());

    ASSERT_TRUE(parsedEventReceived.load()) << "DBCParsedEvent was never published";
    ASSERT_EQ(capturedConfig.messageDefinitions.size(), 1u);

    const auto& sig = capturedConfig.messageDefinitions.front().signalDescriptions.front();
    EXPECT_EQ(sig.signalName, TestHelpers::kTestSignalName.toStdString());
    EXPECT_EQ(sig.startBit, 0u);
    EXPECT_EQ(sig.signalSize, 8u);
    EXPECT_DOUBLE_EQ(sig.factor, 1.0);
    EXPECT_DOUBLE_EQ(sig.offset, 0.0);
    EXPECT_DOUBLE_EQ(sig.minimum, 0.0);
    EXPECT_DOUBLE_EQ(sig.maximum, 255.0);
    EXPECT_EQ(sig.unit, "units");
    EXPECT_TRUE(sig.byteOrder)
        << "Expected byteOrder=true (parser maps @1 → byteOrderInt==1 → true)";
    EXPECT_FALSE(sig.valueType) << "Expected unsigned value type";
}
