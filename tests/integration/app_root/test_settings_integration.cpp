#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QComboBox>
#include <QTest>

#include "app_root/constants.hpp"
#include "app_root/model/settings_model.hpp"
#include "app_root/service/settings_service.hpp"
#include "app_root/view/settings_view.hpp"
#include "core/dto/setting_dto.hpp"
#include "core/event/settings_event.hpp"
#include "core/event/theme_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace AppRoot;
using namespace TestHelpers;
using ::testing::_;
using ::testing::AnyNumber;

// =============================================================================
// Helpers
// =============================================================================

static auto makeThemeSetting()
{
    return std::make_unique<Core::SettingDefinition<
        Core::SettingType::Select, Core::GetAvailableThemesEvent, Core::ThemeChangeEvent>>(
        Core::SettingKey{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID},
        Constants::THEME_ICON_PATH,
        Core::TypeTraits<Core::SettingType::Select, Core::GetAvailableThemesEvent>{"Select Theme"});
}

static auto makeSelectSetting(const std::string& settingId, const std::string& componentId)
{
    return std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{settingId, componentId}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>{
            "Placeholder"});
}

// =============================================================================
// Fixture
// =============================================================================

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

// =============================================================================
// Tests: event round-trip flows
// =============================================================================

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

// SettingsService deduplicates SettingChangedEvent on the same value,
// but SettingsModel always calls publishChanged — so ThemeChangeEvent fires twice.
TEST_F(SettingsIntegrationTest,
       Flow_SetSameValueTwice_ServiceDeduplicates_ModelAlwaysPublishesChanged)
{
    service->registerSetting(makeThemeSetting());

    int settingChangedCount = 0;
    int themeChangedCount = 0;

    auto conn1 = broker->subscribe<Core::SettingChangedEvent<Core::SettingType::Select>>(
        [&](const auto&) { ++settingChangedCount; });
    auto conn2 =
        broker->subscribe<Core::ThemeChangeEvent>([&](const auto&) { ++themeChangedCount; });

    const Core::SettingKey key{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID};
    model->setValue(key, Constants::THEME_DARK);
    model->setValue(key, Constants::THEME_DARK);  // identical value

    // SettingsService deduplicates: SettingChangedEvent published only once.
    EXPECT_EQ(settingChangedCount, 1);
    // SettingsModel always calls publishChanged → ThemeChangeEvent fires both times.
    EXPECT_EQ(themeChangedCount, 2);
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

    // Changing key2 must not affect key1
    model->setValue(key2, "gamma");
    EXPECT_EQ(model->getValue(key1), "alpha");
    EXPECT_EQ(model->getValue(key2), "gamma");
}

// Multiple subscribers to the same provider event all contribute to the options list.
TEST_F(SettingsIntegrationTest, Flow_FetchOptions_MultipleProviders_MergesAllOptions)
{
    service->registerSetting(makeThemeSetting());

    auto conn1 = broker->subscribe<Core::GetAvailableThemesEvent>(
        [](const Core::GetAvailableThemesEvent& e) {
            e.options->push_back({Constants::THEME_LIGHT, Constants::THEME_LIGHT});
        });
    auto conn2 = broker->subscribe<Core::GetAvailableThemesEvent>(
        [](const Core::GetAvailableThemesEvent& e) {
            e.options->push_back({Constants::THEME_DARK, Constants::THEME_DARK});
            e.options->push_back({Constants::THEME_AQUA, Constants::THEME_AQUA});
        });

    const auto settings = model->getSettingsForComponent(Constants::THEME_COMPONENT_ID);
    ASSERT_EQ(settings.size(), 1);

    const auto options = model->fetchOptions(settings.front());

    EXPECT_EQ(options.size(), 3);
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

// Simulates AppRoot::start() registering a theme provider for all 5 themes.
TEST_F(SettingsIntegrationTest, Flow_ThemeProvider_AppRootPattern_FetchReturnsAllFiveThemes)
{
    service->registerSetting(makeThemeSetting());

    // Mirrors what AppRoot::start() subscribes
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
    for (const auto& opt : options)
    {
        EXPECT_EQ(opt.value, expected[i++]);
    }
}

// Component registration order is preserved in getComponentIds.
TEST_F(SettingsIntegrationTest, Flow_ComponentOrder_FirstSeenOrder_PreservedInGetIds)
{
    service->registerSetting(makeSelectSetting("s1", "Comp_B"));
    service->registerSetting(makeSelectSetting("s2", "Comp_A"));
    service->registerSetting(makeSelectSetting("s3", "Comp_B"));  // duplicate component

    const auto ids = model->getComponentIds();

    ASSERT_EQ(ids.size(), 2);
    EXPECT_EQ(ids[0], "Comp_B");  // first seen
    EXPECT_EQ(ids[1], "Comp_A");
}

// fetchOptions on nullptr setting returns an empty list without crashing.
TEST_F(SettingsIntegrationTest, Flow_FetchOptions_NullptrSetting_ReturnsEmptySafely)
{
    const auto result = model->fetchOptions(nullptr);
    EXPECT_TRUE(result.empty());
}

// =============================================================================
// Tests: RAII Connection unsubscribes correctly
// =============================================================================

// Once a Connection goes out of scope, the subscriber no longer receives events.
TEST_F(SettingsIntegrationTest, Flow_Connection_Released_NoLongerReceivesEvents)
{
    service->registerSetting(makeThemeSetting());

    int eventCount = 0;
    {
        auto conn = broker->subscribe<Core::ThemeChangeEvent>(
            [&](const Core::ThemeChangeEvent&) { ++eventCount; });

        const Core::SettingKey key{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID};
        model->setValue(key, Constants::THEME_DARK);
        EXPECT_EQ(eventCount, 1);
        // conn goes out of scope → unsubscribes
    }

    const Core::SettingKey key{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID};
    model->setValue(key, Constants::THEME_AQUA);

    EXPECT_EQ(eventCount, 1);  // no new events after unsubscription
}

// =============================================================================
// Tests: SettingsView rendering
// =============================================================================

// SettingsView creates at least one QComboBox for a registered setting.
TEST_F(SettingsIntegrationTest, Flow_SettingsView_RegisteredSetting_RendersComboBox)
{
    service->registerSetting(makeThemeSetting());

    auto settingsView = std::make_unique<SettingsView>(model.get(), nullptr);
    QTest::qWait(10);

    const auto combos = settingsView->findChildren<QComboBox*>();
    EXPECT_GE(combos.size(), 1);
}

// SettingsView renders one CardWidget section per registered component.
TEST_F(SettingsIntegrationTest, Flow_SettingsView_TwoComponents_TwoCardSections)
{
    service->registerSetting(makeSelectSetting("key1", "ComponentA"));
    service->registerSetting(makeSelectSetting("key2", "ComponentB"));

    auto settingsView = std::make_unique<SettingsView>(model.get(), nullptr);
    QTest::qWait(10);

    const auto cards = settingsView->findChildren<Core::CardWidget*>();
    EXPECT_GE(cards.size(), 2);
}

// Rebuild clears old widgets and re-renders from the current registry state.
// Adding a setting after initial construction must appear after rebuild.
TEST_F(SettingsIntegrationTest, Flow_SettingsView_Rebuild_ReflectsNewlyRegisteredSettings)
{
    service->registerSetting(makeSelectSetting("key1", "ComponentA"));

    auto settingsView = std::make_unique<SettingsView>(model.get(), nullptr);
    QTest::qWait(10);

    const int combosBeforeRebuild = settingsView->findChildren<QComboBox*>().size();

    service->registerSetting(makeSelectSetting("key2", "ComponentA"));
    settingsView->rebuild();
    QTest::qWait(10);

    const int combosAfterRebuild = settingsView->findChildren<QComboBox*>().size();
    EXPECT_GT(combosAfterRebuild, combosBeforeRebuild);
}

// SettingsView with no registered settings renders no combo boxes.
TEST_F(SettingsIntegrationTest, Flow_SettingsView_NoSettings_NoComboBoxes)
{
    auto settingsView = std::make_unique<SettingsView>(model.get(), nullptr);
    QTest::qWait(10);

    const auto combos = settingsView->findChildren<QComboBox*>();
    EXPECT_EQ(combos.size(), 0);
}
