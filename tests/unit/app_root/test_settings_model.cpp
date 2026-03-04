#include <gtest/gtest.h>

#include "app_root/model/settings_model.hpp"
#include "app_root/service/settings_service.hpp"
#include "core/dto/setting_dto.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace AppRoot;
using namespace TestHelpers;

/**
 * @brief Unit tests for SettingsModel.
 */
class SettingsModelTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        mockBroker = std::make_unique<MockEventBroker>();
        service = std::make_unique<SettingsService>(*mockBroker);
        model = std::make_unique<SettingsModel>(*service, *mockBroker);
    }

    std::unique_ptr<MockEventBroker> mockBroker;
    std::unique_ptr<SettingsService> service;
    std::unique_ptr<SettingsModel> model;
};

/**
 * @brief Test model construction.
 */
TEST_F(SettingsModelTest, ConstructsSuccessfully)
{
    EXPECT_NO_THROW(SettingsModel(*service, *mockBroker));
}

/**
 * @brief Test getComponentIds returns empty initially.
 */
TEST_F(SettingsModelTest, GetComponentIdsReturnsEmptyInitially)
{
    const auto ids = model->getComponentIds();
    EXPECT_TRUE(ids.empty());
}

/**
 * @brief Test getComponentIds returns component IDs after registration.
 */
TEST_F(SettingsModelTest, GetComponentIdsReturnsIdsAfterRegistration)
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

    service->registerSetting(std::move(setting1));
    service->registerSetting(std::move(setting2));

    const auto ids = model->getComponentIds();
    EXPECT_EQ(ids.size(), 2);
}

/**
 * @brief Test getSettingsForComponent returns empty for unknown component.
 */
TEST_F(SettingsModelTest, GetSettingsForComponentReturnsEmptyForUnknown)
{
    const auto settings = model->getSettingsForComponent("unknown");
    EXPECT_TRUE(settings.empty());
}

/**
 * @brief Test getSettingsForComponent returns settings after registration.
 */
TEST_F(SettingsModelTest, GetSettingsForComponentReturnsSettingsAfterRegistration)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key1", "component1"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder"));

    service->registerSetting(std::move(setting));

    const auto settings = model->getSettingsForComponent("component1");
    EXPECT_EQ(settings.size(), 1);
}

/**
 * @brief Test getValue returns default value.
 */
TEST_F(SettingsModelTest, GetValueReturnsDefaultValue)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key", "component"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder"));

    service->registerSetting(std::move(setting));

    const auto value = model->getValue(Core::SettingKey{"key", "component"});
    EXPECT_TRUE(value.empty());
}

/**
 * @brief Test setValue updates value.
 */
TEST_F(SettingsModelTest, SetValueUpdatesValue)
{
    auto setting = std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{"key", "component"}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>(
            "Placeholder"));

    service->registerSetting(std::move(setting));

    model->setValue(Core::SettingKey{"key", "component"}, "new_value");

    EXPECT_EQ(model->getValue(Core::SettingKey{"key", "component"}), "new_value");
}

/**
 * @brief Test getValue returns empty for non-existent key.
 */
TEST_F(SettingsModelTest, GetValueReturnsEmptyForNonExistentKey)
{
    const auto value = model->getValue(Core::SettingKey{"nonexistent", "key"});
    EXPECT_TRUE(value.empty());
}

/**
 * @brief Test setValue for non-existent key does not crash.
 */
TEST_F(SettingsModelTest, SetValueForNonExistentKeyDoesNotCrash)
{
    EXPECT_NO_THROW(model->setValue(Core::SettingKey{"nonexistent", "key"}, "value"));
}

/**
 * @brief Test getSettingsForComponent filters by component ID.
 */
TEST_F(SettingsModelTest, GetSettingsForComponentFiltersCorrectly)
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

    const auto settings1 = model->getSettingsForComponent("component1");
    EXPECT_EQ(settings1.size(), 2);

    const auto settings2 = model->getSettingsForComponent("component2");
    EXPECT_EQ(settings2.size(), 1);
}