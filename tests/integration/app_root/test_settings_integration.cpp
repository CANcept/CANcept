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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QComboBox>
#include <QTest>

#include "app_root/constants.hpp"
#include "app_root/model/settings_model.hpp"
#include "app_root/service/settings_service.hpp"
#include "app_root/view/settings_view.hpp"
#include "core/event/theme_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/setting_definition_builder.hpp"

using namespace AppRoot;
using namespace TestHelpers;
using ::testing::_;
using ::testing::AnyNumber;

class SettingsIntegrationTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        broker = std::make_shared<MockEventBroker>();
        EXPECT_CALL(*broker, _subscribeEvent(_)).Times(AnyNumber());
        EXPECT_CALL(*broker, _publishEvent(_, _)).Times(AnyNumber());

        service = std::make_unique<SettingsService>(*broker);
        model = std::make_unique<SettingsModel>(*service, *broker);
    }

    void TearDown() override
    {
        model.reset();
        service.reset();
        broker.reset();
    }

    std::shared_ptr<MockEventBroker> broker;
    std::unique_ptr<SettingsService> service;
    std::unique_ptr<SettingsModel> model;
};

// Setting a new theme value via the model publishes ThemeChangeEvent on the broker.
TEST_F(SettingsIntegrationTest, Flow_ThemeChange_SetValueThroughModel_PublishesThemeChangeEvent)
{
    service->registerSetting(makeThemeSetting());

    std::string capturedTheme;
    auto conn = broker->subscribe<Core::ThemeChangeEvent>(
        [&](const Core::ThemeChangeEvent& e) { capturedTheme = e.themeName; });

    const Core::SettingKey key{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID};
    model->setValue(key, Constants::THEME_DARK);

    EXPECT_EQ(capturedTheme, Constants::THEME_DARK);
}

// Both the published event and the stored value reflect the updated theme.
TEST_F(SettingsIntegrationTest, Flow_ThemeChange_EventAndStoredValueBothReflectNewTheme)
{
    service->registerSetting(makeThemeSetting());

    std::string capturedTheme;
    auto conn = broker->subscribe<Core::ThemeChangeEvent>(
        [&](const Core::ThemeChangeEvent& e) { capturedTheme = e.themeName; });

    const Core::SettingKey key{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID};
    model->setValue(key, Constants::THEME_AQUA);

    EXPECT_EQ(capturedTheme, Constants::THEME_AQUA);
    EXPECT_EQ(model->getValue(key), Constants::THEME_AQUA);
}

// fetchOptions publishes the provider event; the subscriber fills the options list.
TEST_F(SettingsIntegrationTest, Flow_FetchOptions_CollectorPattern_SubscriberFillsList)
{
    service->registerSetting(makeThemeSetting());

    auto conn = broker->subscribe<Core::GetAvailableThemesEvent>(
        [](const Core::GetAvailableThemesEvent& e) {
            e.options->push_back({Constants::THEME_LIGHT, Constants::THEME_LIGHT});
            e.options->push_back({Constants::THEME_DARK, Constants::THEME_DARK});
        });

    const auto settings = model->getSettingsForComponent(Constants::THEME_COMPONENT_ID);
    ASSERT_EQ(settings.size(), 1);

    const auto options = model->fetchOptions(settings.front());

    ASSERT_EQ(options.size(), 2);
    EXPECT_EQ(options.front().value, Constants::THEME_LIGHT);
    EXPECT_EQ(std::next(options.begin())->value, Constants::THEME_DARK);
}

// Settings in two different components are completely independent.
TEST_F(SettingsIntegrationTest, Flow_MultiComponent_IndependentValues_NoCrossContamination)
{
    service->registerSetting(makeSelectSetting("ThemeKey", "Component1"));
    service->registerSetting(makeSelectSetting("LayoutKey", "Component2"));

    const Core::SettingKey key1{"ThemeKey", "Component1"};
    const Core::SettingKey key2{"LayoutKey", "Component2"};

    model->setValue(key1, "alpha");
    model->setValue(key2, "beta");

    EXPECT_EQ(model->getValue(key1), "alpha");
    EXPECT_EQ(model->getValue(key2), "beta");

    model->setValue(key2, "gamma");
    EXPECT_EQ(model->getValue(key1), "alpha");
    EXPECT_EQ(model->getValue(key2), "gamma");
}

// Cycling through all 5 themes publishes ThemeChangeEvent each time with the correct name.
TEST_F(SettingsIntegrationTest, Flow_ThemeChange_FullCycle_AllFiveThemes_PublishInOrder)
{
    service->registerSetting(makeThemeSetting());

    std::vector<std::string> received;
    auto conn = broker->subscribe<Core::ThemeChangeEvent>(
        [&](const Core::ThemeChangeEvent& e) { received.push_back(e.themeName); });

    const Core::SettingKey key{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID};
    const std::vector<std::string> sequence = {Constants::THEME_DARK, Constants::THEME_AQUA,
                                               Constants::THEME_MAROON, Constants::THEME_DRACULA,
                                               Constants::THEME_LIGHT};
    for (const auto& theme : sequence)
    {
        model->setValue(key, theme);
    }

    ASSERT_EQ(received.size(), 5);
    EXPECT_EQ(received[0], Constants::THEME_DARK);
    EXPECT_EQ(received[2], Constants::THEME_MAROON);
    EXPECT_EQ(received[4], Constants::THEME_LIGHT);
}

// Simulates AppRoot::start()
TEST_F(SettingsIntegrationTest, Flow_ThemeProvider_AppRootPattern_FetchReturnsAllFiveThemes)
{
    service->registerSetting(makeThemeSetting());

    auto conn = broker->subscribe<Core::GetAvailableThemesEvent>(
        [](const Core::GetAvailableThemesEvent& e) {
            e.options->push_back({Constants::THEME_LIGHT, Constants::THEME_LIGHT});
            e.options->push_back({Constants::THEME_DARK, Constants::THEME_DARK});
            e.options->push_back({Constants::THEME_AQUA, Constants::THEME_AQUA});
            e.options->push_back({Constants::THEME_MAROON, Constants::THEME_MAROON});
            e.options->push_back({Constants::THEME_DRACULA, Constants::THEME_DRACULA});
        });

    const auto settings = model->getSettingsForComponent(Constants::THEME_COMPONENT_ID);
    ASSERT_EQ(settings.size(), 1);

    const auto options = model->fetchOptions(settings.front());

    ASSERT_EQ(options.size(), 5);
    const std::vector<std::string> expected = {Constants::THEME_LIGHT, Constants::THEME_DARK,
                                               Constants::THEME_AQUA, Constants::THEME_MAROON,
                                               Constants::THEME_DRACULA};
    int i = 0;
    for (const auto& [value, displayText] : options)
    {
        EXPECT_EQ(value, expected[i++]);
    }
}

// Component registration order is preserved in getComponentIds.
TEST_F(SettingsIntegrationTest, Flow_ComponentOrder_FirstSeenOrder_PreservedInGetIds)
{
    service->registerSetting(makeSelectSetting("s1", "Comp_B"));
    service->registerSetting(makeSelectSetting("s2", "Comp_A"));
    service->registerSetting(makeSelectSetting("s3", "Comp_B"));

    const auto ids = model->getComponentIds();

    ASSERT_EQ(ids.size(), 2);
    EXPECT_EQ(ids[0], "Comp_B");
    EXPECT_EQ(ids[1], "Comp_A");
}

// fetchOptions on nullptr setting returns an empty list without crashing.
TEST_F(SettingsIntegrationTest, Flow_FetchOptions_NullptrSetting_ReturnsEmptySafely)
{
    const auto result = model->fetchOptions(nullptr);
    EXPECT_TRUE(result.empty());
}

// SettingsView creates at least one QComboBox for a registered setting.
TEST_F(SettingsIntegrationTest, Flow_SettingsView_RegisteredSetting_RendersComboBox)
{
    service->registerSetting(makeThemeSetting());

    const auto settingsView = std::make_unique<SettingsView>(model.get(), nullptr);
    QTest::qWait(10);

    const auto combos = settingsView->findChildren<QComboBox*>();
    EXPECT_GE(combos.size(), 1);
}

// SettingsView renders one CardWidget section per registered component.
TEST_F(SettingsIntegrationTest, Flow_SettingsView_TwoComponents_TwoCardSections)
{
    service->registerSetting(makeSelectSetting("key1", "ComponentA"));
    service->registerSetting(makeSelectSetting("key2", "ComponentB"));

    const auto settingsView = std::make_unique<SettingsView>(model.get(), nullptr);
    QTest::qWait(10);

    const auto cards = settingsView->findChildren<Core::CardWidget*>();
    EXPECT_GE(cards.size(), 2);
}

// Rebuild clears old widgets and re-renders from the current registry state.
TEST_F(SettingsIntegrationTest, Flow_SettingsView_Rebuild_ReflectsNewlyRegisteredSettings)
{
    service->registerSetting(makeSelectSetting("key1", "ComponentA"));

    const auto settingsView = std::make_unique<SettingsView>(model.get(), nullptr);
    QTest::qWait(10);

    const int combosBeforeRebuild = settingsView->findChildren<QComboBox*>().size();

    service->registerSetting(makeSelectSetting("key2", "ComponentA"));
    settingsView->rebuild();
    QTest::qWait(10);

    const int combosAfterRebuild = settingsView->findChildren<QComboBox*>().size();
    EXPECT_GT(combosAfterRebuild, combosBeforeRebuild);
}
