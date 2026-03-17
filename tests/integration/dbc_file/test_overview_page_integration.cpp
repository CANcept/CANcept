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

#include "dbc_file/constants.hpp"
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/pages/overview_page.hpp"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace DbcFile;
using namespace testing;

class OverviewPageIntegrationTest : public ::testing::Test
{
   protected:
    int argc = 0;
    char** argv = nullptr;
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<DbcComponent> component;
    OverviewPage* overviewPage = nullptr;

    void SetUp() override
    {
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }
        EXPECT_CALL(mockBroker, _subscribeEvent(_)).WillRepeatedly(Return());

        component = std::make_unique<DbcComponent>(mockBroker);
        component->onStart();

        QWidget* mainView = component->getView();
        overviewPage = mainView->findChild<OverviewPage*>();
        ASSERT_NE(overviewPage, nullptr);
    }

    void TearDown() override
    {
        component->onStop();
        component.reset();
    }

    QLabel* getLabel(const QString& objectName)
    {
        return overviewPage->findChild<QLabel*>(objectName);
    }
};

/**
 * @brief Test: Metadata
 */
TEST_F(OverviewPageIntegrationTest, DisplaysFileMetadata)
{
    // 1. Arrange
    const std::string testFile = "integration_test.dbc";
    std::string testVersion = "2.0";

    auto config = TestHelpers::DbcConfigBuilder().fileName(testFile).version(testVersion).build();

    // 2. Act
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "path"));

    // 3. Assert
    QLabel* lblFile = getLabel(Constants::OverviewPage::LblNames::FileName);
    QLabel* lblVersion = getLabel(Constants::OverviewPage::LblNames::Version);

    ASSERT_NE(lblFile, nullptr) << "FileName label not found";
    ASSERT_NE(lblVersion, nullptr) << "Version-Label not found";

    EXPECT_EQ(lblFile->text(), QString::fromStdString(testFile));
    EXPECT_EQ(lblVersion->text(), QString::fromStdString(testVersion));
}

/**
 * @brief Test: Counter labels
 */
TEST_F(OverviewPageIntegrationTest, DisplaysCorrectStatistics)
{
    // 1. Arrange: Config mit 1 Node, 2 Messages, 3 Signals
    auto config = TestHelpers::DbcConfigBuilder()
                      .node("Engine")  // 1 ECU
                      .message(TestHelpers::DbcMessageBuilder(0x100, "A")
                                   .transmitter("Engine")
                                   .signal(TestHelpers::DbcSignalBuilder("S1")))  // +1 Signal
                      .message(TestHelpers::DbcMessageBuilder(0x200, "B")
                                   .transmitter("Unknown")
                                   .signal(TestHelpers::DbcSignalBuilder("S2"))
                                   .signal(TestHelpers::DbcSignalBuilder("S3")))  // +2 Signals
                      .build();

    // 2. Act
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "path"));

    // 3. Assert
    QLabel* lblEcu = getLabel(Constants::OverviewPage::LblNames::EcuCount);
    QLabel* lblMsg = getLabel(Constants::OverviewPage::LblNames::MsgCount);
    QLabel* lblSig = getLabel(Constants::OverviewPage::LblNames::SigCount);
    QLabel* lblOrphans = getLabel(Constants::OverviewPage::LblNames::OrphanCount);

    ASSERT_NE(lblEcu, nullptr);
    ASSERT_NE(lblMsg, nullptr);
    ASSERT_NE(lblSig, nullptr);
    ASSERT_NE(lblOrphans, nullptr);

    EXPECT_EQ(lblEcu->text(), "1");
    EXPECT_EQ(lblMsg->text(), "2");
    EXPECT_EQ(lblSig->text(), "3");
    EXPECT_EQ(lblOrphans->text(), "1");
}

/**
 * @brief Test: List-content (for ecus and messages overview lists)
 */
TEST_F(OverviewPageIntegrationTest, PopulatesOverviewLists)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .node("TestECU")
                      .message(TestHelpers::DbcMessageBuilder(0x100, "MsgA").transmitter("TestECU"))
                      .message(TestHelpers::DbcMessageBuilder(0x200, "MsgB").transmitter("Unknown"))
                      .build();

    // 2. Act
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "path"));

    // 3. Assert
    QListView* ecuList = overviewPage->getEcuList();
    QListView* msgList = overviewPage->getMessageList();

    ASSERT_NE(ecuList, nullptr);
    ASSERT_NE(ecuList->model(), nullptr);

    ASSERT_NE(msgList, nullptr);
    ASSERT_NE(msgList->model(), nullptr);

    EXPECT_EQ(ecuList->model()->index(0, 0).data().toString(), "TestECU");
    EXPECT_EQ(msgList->model()->index(0, 0).data().toString(), "MsgA");
    EXPECT_EQ(msgList->model()->index(1, 0).data().toString(), "MsgB");
}