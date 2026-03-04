#include <gtest/gtest.h>

#include <memory>

#include "app_root/entry_point/tab_factory.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/mock_tab_component.hpp"

using namespace AppRoot;
using namespace TestHelpers;

// Another mock for testing multiple types
class AnotherMockTab : public Core::ITabComponent
{
   public:
    explicit AnotherMockTab(Core::IEventBroker& broker, QString id = "another_mock",
                            QString title = "Another Mock", QIcon icon = QIcon())
        : ITabComponent(broker, std::move(id), std::move(title), std::move(icon))
    {
    }

    ~AnotherMockTab() override = default;

    void onStart() override {}
    void onStop() override {}
    auto getView() -> QWidget* override
    {
        return nullptr;
    }
};

/**
 * @brief Unit tests for TabFactory.
 */
class TabFactoryTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        mockBroker = std::make_unique<MockEventBroker>();
    }

    std::unique_ptr<MockEventBroker> mockBroker;
    TabFactory factory;
};

/**
 * @brief Test factory construction.
 */
TEST_F(TabFactoryTest, ConstructsSuccessfully)
{
    EXPECT_NO_THROW(TabFactory());
}

/**
 * @brief Test registering a creator.
 */
TEST_F(TabFactoryTest, RegistersCreator)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });

    EXPECT_TRUE(factory.isRegistered(std::type_index(typeid(MockTabComponent))));
}

/**
 * @brief Test creating an instance after registration.
 */
TEST_F(TabFactoryTest, CreatesInstanceAfterRegistration)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });

    const auto instance = factory.create<MockTabComponent>();

    ASSERT_NE(instance, nullptr);
}

/**
 * @brief Test create returns nullptr for unregistered type.
 */
TEST_F(TabFactoryTest, ReturnsNullptrForUnregisteredType)
{
    auto instance = factory.create<MockTabComponent>();

    EXPECT_EQ(instance, nullptr);
}

/**
 * @brief Test isRegistered returns false for unregistered type.
 */
TEST_F(TabFactoryTest, IsRegisteredReturnsFalseForUnregisteredType)
{
    EXPECT_FALSE(factory.isRegistered(std::type_index(typeid(MockTabComponent))));
}

/**
 * @brief Test registering multiple creators.
 */
TEST_F(TabFactoryTest, RegistersMultipleCreators)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });
    factory.registerCreator<AnotherMockTab>(
        [this]() { return std::make_unique<AnotherMockTab>(*mockBroker); });

    EXPECT_TRUE(factory.isRegistered(std::type_index(typeid(MockTabComponent))));
    EXPECT_TRUE(factory.isRegistered(std::type_index(typeid(AnotherMockTab))));
}

/**
 * @brief Test creating multiple types.
 */
TEST_F(TabFactoryTest, CreatesMultipleTypes)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });
    factory.registerCreator<AnotherMockTab>(
        [this]() { return std::make_unique<AnotherMockTab>(*mockBroker); });

    const auto instance1 = factory.create<MockTabComponent>();
    const auto instance2 = factory.create<AnotherMockTab>();

    ASSERT_NE(instance1, nullptr);
    ASSERT_NE(instance2, nullptr);
}

/**
 * @brief Test createByTypeIndex with valid type.
 */
TEST_F(TabFactoryTest, CreateByTypeIndexWithValidType)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });

    const auto instance = factory.createByTypeIndex(std::type_index(typeid(MockTabComponent)));

    ASSERT_NE(instance, nullptr);
}

/**
 * @brief Test createByTypeIndex with invalid type.
 */
TEST_F(TabFactoryTest, CreateByTypeIndexReturnsNullptrForInvalidType)
{
    const auto instance = factory.createByTypeIndex(std::type_index(typeid(MockTabComponent)));

    EXPECT_EQ(instance, nullptr);
}

/**
 * @brief Test canRestart initially returns true.
 */
TEST_F(TabFactoryTest, CanRestartInitiallyReturnsTrue)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });

    EXPECT_TRUE(factory.canRestart(std::type_index(typeid(MockTabComponent))));
}

/**
 * @brief Test canRestart decrements counter.
 */
TEST_F(TabFactoryTest, CanRestartDecrementsCounter)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });

    // DEFAULT_RESTART_LIMIT is 2, so we can restart twice
    EXPECT_TRUE(factory.canRestart(std::type_index(typeid(MockTabComponent))));
    EXPECT_TRUE(factory.canRestart(std::type_index(typeid(MockTabComponent))));
    EXPECT_FALSE(factory.canRestart(std::type_index(typeid(MockTabComponent))));
}

/**
 * @brief Test canRestart returns false for unregistered type.
 */
TEST_F(TabFactoryTest, CanRestartReturnsFalseForUnregisteredType)
{
    EXPECT_FALSE(factory.canRestart(std::type_index(typeid(MockTabComponent))));
}

/**
 * @brief Test creating multiple instances from same creator.
 */
TEST_F(TabFactoryTest, CreatesMultipleInstancesFromSameCreator)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });

    const auto instance1 = factory.create<MockTabComponent>();
    const auto instance2 = factory.create<MockTabComponent>();

    ASSERT_NE(instance1, nullptr);
    ASSERT_NE(instance2, nullptr);
    EXPECT_NE(instance1.get(), instance2.get());
}

/**
 * @brief Test restart limit is independent per type.
 */
TEST_F(TabFactoryTest, RestartLimitIsIndependentPerType)
{
    factory.registerCreator<MockTabComponent>(
        [this]() { return std::make_unique<MockTabComponent>(*mockBroker); });
    factory.registerCreator<AnotherMockTab>(
        [this]() { return std::make_unique<AnotherMockTab>(*mockBroker); });

    factory.canRestart(std::type_index(typeid(MockTabComponent)));
    factory.canRestart(std::type_index(typeid(MockTabComponent)));

    EXPECT_TRUE(factory.canRestart(std::type_index(typeid(AnotherMockTab))));
}