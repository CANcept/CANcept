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

#include <QApplication>

// Module Includes
#include "dbc_file/constants.hpp"
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/pages/load_page.hpp"

// Test Helpers
#include "tests/helpers/mock_event_broker.hpp"

using namespace DbcFile;
using namespace testing;

class LoadPageIntegrationTest : public ::testing::Test
{
   protected:
    int argc = 0;
    char** argv = nullptr;
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;

    std::unique_ptr<DbcComponent> component;

    LoadPage* loadPage = nullptr;

    void SetUp() override
    {
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }

        component = std::make_unique<DbcComponent>(mockBroker);
        component->onStart();

        QWidget* mainView = component->getView();
        loadPage = mainView->findChild<LoadPage*>();

        ASSERT_NE(loadPage, nullptr) << "LoadPage widget could not be found in DbcView";
    }

    void TearDown() override
    {
        component->onStop();
        component.reset();
    }

    QLabel* getStatusLabel()
    {
        return loadPage->findChild<QLabel*>("StatusLabel");
    }
};

/**
 * @brief Test 1: Data Flow OUT (User Action -> Event)
 *
 * Scenario: User selects file
 * Expectation: Component sends parse request signal
 */
TEST_F(LoadPageIntegrationTest, EmitsParseRequestOnUserSelection)
{
    const std::string testFilePath = "/path/to/can.dbc";

    EXPECT_CALL(mockBroker,
                _publishEvent(Matcher<std::type_index>(typeid(Core::ParseDBCRequestEvent)), _))
        .Times(1);

    // Simulate file selection
    emit loadPage->fileSelected(QString::fromStdString(testFilePath));
}

/**
 * @brief Test 2: Data Flow IN (Error Event -> UI Update)
 *
 * Scenario: Backend reports parse error
 * Expectation: Loadpage shows error message
 */
TEST_F(LoadPageIntegrationTest, DisplaysErrorMessageOnParseFailure)
{
    // 1. Arrange
    const std::string errorText = "File not found";
    Core::DBCParseErrorEvent event(errorText, "test.dbc");

    // 2. Act
    mockBroker.triggerEvent(event);

    // 3. Assert
    QLabel* statusLabel = getStatusLabel();

    ASSERT_NE(statusLabel, nullptr) << "Status Label not found!";
    EXPECT_TRUE(statusLabel->text().contains("Error"));
}

/**
 * @brief Test 3: Data Flow IN (Success Event -> UI Update)
 *
 * Scenario: Backend reports parse success
 * Expectation: LoadPage shows success message, sidebar unlocks.
 */
TEST_F(LoadPageIntegrationTest, DisplaysSuccessMessageOnParseSuccess)
{
    // 1. Arrange
    Core::DbcConfig emptyConfig;
    Core::DBCParsedEvent event(emptyConfig, "test.dbc");

    // 2. Act
    mockBroker.triggerEvent(event);

    // 3. Assert
    QLabel* statusLabel = getStatusLabel();
    ASSERT_NE(statusLabel, nullptr);
    EXPECT_TRUE(statusLabel->text().contains("successful"));
}
