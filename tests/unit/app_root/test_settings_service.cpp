#include <gtest/gtest.h>

#include "app_root/service/settings_service.hpp"
#include "core/dto/setting_dto.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace AppRoot;
using namespace TestHelpers;

/**
 * @brief Unit tests for SettingsService.
 */
class SettingsServiceTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        mockBroker = std::make_unique<MockEventBroker>();
        service = std::make_unique<SettingsService>(*mockBroker);
    }

    std::unique_ptr<MockEventBroker> mockBroker;
    std::unique_ptr<SettingsService> service;
};

/**
 * @brief Test service construction.
 */
TEST_F(SettingsServiceTest, ConstructsSuccessfully)
{
    EXPECT_NO_THROW(SettingsService(*mockBroker));
}

/**
 * @brief Test getSettings returns empty initially.
 */
TEST_F(SettingsServiceTest, GetSettingsReturnsEmptyInitially)
{
    const auto& settings = service->getSettings();
    EXPECT_TRUE(settings.empty());
}

/**
 * @brief Test getComponentIds returns empty initially.
 */
TEST_F(SettingsServiceTest, GetComponentIdsReturnsEmptyInitially)
{
    const auto ids = service->getComponentIds();
    EXPECT_TRUE(ids.empty());
}

/**
 * @brief Test registerSetting adds a setting.
 */
TEST_F(SettingsServiceTest, RegisterSettingAddsSetting)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key1", "test.component"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Select an option"));

    service->registerSetting(std::move(setting));

    const auto& settings = service->getSettings();
    EXPECT_EQ(settings.size(), 1);
}

/**
 * @brief Test getValue returns default for unset setting.
 */
TEST_F(SettingsServiceTest, GetValueReturnsDefaultForUnsetSetting)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key1", "test.component"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Select an option"));

    const Core::SettingKey key{"test.component", "key1"};
    service->registerSetting(std::move(setting));

    const auto value = service->getValue(key);
    EXPECT_TRUE(value.empty());
}

/**
 * @brief Test setValue updates value.
 */
TEST_F(SettingsServiceTest, SetValueUpdatesValue)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key1", "test.component"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Select an option"));

    const Core::SettingKey key{"test.component", "key1"};
    service->registerSetting(std::move(setting));

    service->setValue(key, "new_value");

    EXPECT_EQ(service->getValue(key), "new_value");
}

/**
 * @brief Test getSettingsByComponent returns settings for component.
 */
TEST_F(SettingsServiceTest, GetSettingsByComponentReturnsComponentSettings)
{
    auto setting1 = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key1", "component1"}, "icon1",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder 1"));
    auto setting2 = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key2", "component1"}, "icon2",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder 2"));
    auto setting3 = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key3", "component2"}, "icon3",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder 3"));

    service->registerSetting(std::move(setting1));
    service->registerSetting(std::move(setting2));
    service->registerSetting(std::move(setting3));

    const auto component1Settings = service->getSettingsByComponent("component1");
    EXPECT_EQ(component1Settings.size(), 2);

    const auto component2Settings = service->getSettingsByComponent("component2");
    EXPECT_EQ(component2Settings.size(), 1);
}

/**
 * @brief Test getComponentIds returns unique component IDs in order.
 */
TEST_F(SettingsServiceTest, GetComponentIdsReturnsUniqueIdsInOrder)
{
    auto setting1 = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key1", "component1"}, "icon1",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder 1"));
    auto setting2 = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key2", "component2"}, "icon2",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder 2"));
    auto setting3 = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key3", "component1"}, "icon3",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder 3"));

    service->registerSetting(std::move(setting1));
    service->registerSetting(std::move(setting2));
    service->registerSetting(std::move(setting3));

    const auto ids = service->getComponentIds();
    EXPECT_EQ(ids.size(), 2);
    EXPECT_EQ(ids[0], "component1");
    EXPECT_EQ(ids[1], "component2");
}

/**
 * @brief Test getValue returns empty for non-existent setting.
 */
TEST_F(SettingsServiceTest, GetValueReturnsEmptyForNonExistentSetting)
{
    const Core::SettingKey key{"nonexistent", "key"};
    const auto value = service->getValue(key);
    EXPECT_TRUE(value.empty());
}

/**
 * @brief Test setValue for non-existent setting does nothing.
 */
TEST_F(SettingsServiceTest, SetValueForNonExistentSettingDoesNothing)
{
    const Core::SettingKey key{"nonexistent", "key"};
    EXPECT_NO_THROW(service->setValue(key, "value"));
}

/**
 * @brief Test multiple setValue calls update value correctly.
 */
TEST_F(SettingsServiceTest, MultipleSetValueCallsUpdateCorrectly)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key", "test"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Select an option"));

    const Core::SettingKey key{"test", "key"};
    service->registerSetting(std::move(setting));

    service->setValue(key, "value1");
    EXPECT_EQ(service->getValue(key), "value1");

    service->setValue(key, "value2");
    EXPECT_EQ(service->getValue(key), "value2");

    service->setValue(key, "value3");
    EXPECT_EQ(service->getValue(key), "value3");
}

/**
 * @brief Test getSettingsByComponent returns empty for unknown component.
 */
TEST_F(SettingsServiceTest, GetSettingsByComponentReturnsEmptyForUnknownComponent)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key1", "component1"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Select an option"));

    service->registerSetting(std::move(setting));

    const auto settings = service->getSettingsByComponent("unknown");
    EXPECT_TRUE(settings.empty());
}

/**
 * @brief Test registering multiple settings for same component.
 */
TEST_F(SettingsServiceTest, RegistersMultipleSettingsForSameComponent)
{
    for (int i = 0; i < 5; ++i)
    {
        auto setting = std::make_unique<
            Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
            Core::SettingKey{"key" + std::to_string(i), "component"}, "icon",
            Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
                "Select an option"));
        service->registerSetting(std::move(setting));
    }

    const auto settings = service->getSettingsByComponent("component");
    EXPECT_EQ(settings.size(), 5);
}
