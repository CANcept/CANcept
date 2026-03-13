#include <gtest/gtest.h>

#include <QPixmap>
#include <QSignalSpy>

#include "app_root/model/app_root_model.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/mock_tab_component.hpp"

using namespace AppRoot;
using namespace TestHelpers;

/**
 * @brief Unit tests for AppRootModel.
 */
class AppRootModelTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        mockBroker = std::make_unique<MockEventBroker>();
        model = std::make_unique<AppRootModel>();
        tab1 = std::make_unique<MockTabComponent>(*mockBroker, "tab1", "Tab 1");
        tab2 = std::make_unique<MockTabComponent>(*mockBroker, "tab2", "Tab 2");
        tab3 = std::make_unique<MockTabComponent>(*mockBroker, "tab3", "Tab 3");
    }

    std::unique_ptr<MockEventBroker> mockBroker;
    std::unique_ptr<AppRootModel> model;
    std::unique_ptr<MockTabComponent> tab1;
    std::unique_ptr<MockTabComponent> tab2;
    std::unique_ptr<MockTabComponent> tab3;
};

/**
 * @brief Test model construction.
 */
TEST_F(AppRootModelTest, ConstructsSuccessfully)
{
    EXPECT_NO_THROW(AppRootModel());
}

/**
 * @brief Test rowCount returns 0 initially.
 */
TEST_F(AppRootModelTest, RowCountReturnsZeroInitially)
{
    EXPECT_EQ(model->rowCount(), 0);
}

/**
 * @brief Test addTab increases row count.
 */
TEST_F(AppRootModelTest, AddTabIncreasesRowCount)
{
    model->addTab(tab1.get());
    EXPECT_EQ(model->rowCount(), 1);

    model->addTab(tab2.get());
    EXPECT_EQ(model->rowCount(), 2);
}

/**
 * @brief Test componentAt returns correct component.
 */
TEST_F(AppRootModelTest, ComponentAtReturnsCorrectComponent)
{
    model->addTab(tab1.get());
    model->addTab(tab2.get());

    EXPECT_EQ(model->componentAt(0), tab1.get());
    EXPECT_EQ(model->componentAt(1), tab2.get());
}

/**
 * @brief Test componentAt returns nullptr for out of bounds.
 */
TEST_F(AppRootModelTest, ComponentAtReturnsNullptrForOutOfBounds)
{
    model->addTab(tab1.get());

    EXPECT_EQ(model->componentAt(-1), nullptr);
    EXPECT_EQ(model->componentAt(1), nullptr);
    EXPECT_EQ(model->componentAt(100), nullptr);
}

/**
 * @brief Test data returns valid data for ComponentRole.
 */
TEST_F(AppRootModelTest, DataReturnsComponentForComponentRole)
{
    model->addTab(tab1.get());

    const QModelIndex idx = model->index(0, 0);
    const QVariant data = model->data(idx, AppRootModel::ComponentRole);

    EXPECT_TRUE(data.isValid());
    auto* component = data.value<Core::ITabComponent*>();
    EXPECT_EQ(component, tab1.get());
}

/**
 * @brief Test data returns invalid for invalid index.
 */
TEST_F(AppRootModelTest, DataReturnsInvalidForInvalidIndex)
{
    constexpr QModelIndex invalidIdx;
    const QVariant data = model->data(invalidIdx, AppRootModel::ComponentRole);

    EXPECT_FALSE(data.isValid());
}

/**
 * @brief Test data returns invalid for out of bounds row.
 */
TEST_F(AppRootModelTest, DataReturnsInvalidForOutOfBoundsRow)
{
    model->addTab(tab1.get());

    const QModelIndex idx = model->index(10, 0);
    const QVariant data = model->data(idx, AppRootModel::ComponentRole);

    EXPECT_FALSE(data.isValid());
}

/**
 * @brief Test replaceTab replaces tab correctly.
 */
TEST_F(AppRootModelTest, ReplaceTabReplacesCorrectly)
{
    model->addTab(tab1.get());
    model->addTab(tab2.get());

    model->replaceTab(tab1.get(), tab3.get());

    EXPECT_EQ(model->componentAt(0), tab3.get());
    EXPECT_EQ(model->componentAt(1), tab2.get());
    EXPECT_EQ(model->rowCount(), 2);
}

/**
 * @brief Test replaceTab with non-existent old tab does nothing.
 */
TEST_F(AppRootModelTest, ReplaceTabWithNonExistentOldTabDoesNothing)
{
    model->addTab(tab1.get());

    const int initialCount = model->rowCount();
    model->replaceTab(tab2.get(), tab3.get());

    EXPECT_EQ(model->rowCount(), initialCount);
    EXPECT_EQ(model->componentAt(0), tab1.get());
}

/**
 * @brief Test index creates valid index within bounds.
 */
TEST_F(AppRootModelTest, IndexCreatesValidIndexWithinBounds)
{
    model->addTab(tab1.get());

    const QModelIndex idx = model->index(0, 0);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);
    EXPECT_EQ(idx.column(), 0);
}

/**
 * @brief Test index creates invalid index for out of bounds.
 */
TEST_F(AppRootModelTest, IndexCreatesInvalidIndexForOutOfBounds)
{
    model->addTab(tab1.get());

    const QModelIndex idx = model->index(10, 0);
    EXPECT_FALSE(idx.isValid());
}

/**
 * @brief Test componentAt returns nullptr for empty model.
 */
TEST_F(AppRootModelTest, ComponentAtReturnsNullptrForEmptyModel)
{
    EXPECT_EQ(model->componentAt(0), nullptr);
}

/**
 * @brief Test data returns invalid for empty model.
 */
TEST_F(AppRootModelTest, DataReturnsInvalidForEmptyModel)
{
    const QModelIndex idx = model->index(0, 0);
    const QVariant data = model->data(idx, AppRootModel::ComponentRole);

    EXPECT_FALSE(data.isValid());
}

/**
 * @brief Test replaceTab preserves order.
 */
TEST_F(AppRootModelTest, ReplaceTabPreservesOrder)
{
    model->addTab(tab1.get());
    model->addTab(tab2.get());

    model->replaceTab(tab1.get(), tab3.get());

    EXPECT_EQ(model->componentAt(0), tab3.get());
    EXPECT_EQ(model->componentAt(1), tab2.get());
}

/**
 * @brief DecorationRole returns the icon set on the component.
 */
TEST_F(AppRootModelTest, DataReturnsIconForDecorationRole)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);
    auto tabWithIcon =
        std::make_unique<MockTabComponent>(*mockBroker, "icon_tab", "Icon Tab", QIcon(pixmap));
    model->addTab(tabWithIcon.get());

    const QVariant result = model->data(model->index(0, 0), Qt::DecorationRole);

    EXPECT_TRUE(result.isValid());
    EXPECT_FALSE(result.value<QIcon>().isNull());
}

/**
 * @brief Emitting updated() on a tab causes the model to emit dataChanged for that row.
 */
TEST_F(AppRootModelTest, UpdatedSignal_EmitsDataChangedForCorrectRow)
{
    model->addTab(tab1.get());
    model->addTab(tab2.get());

    QSignalSpy spy(model.get(), &AppRootModel::dataChanged);

    emit tab2->updated();

    ASSERT_EQ(spy.count(), 1);
    const QModelIndex changedIndex = spy.at(0).at(0).value<QModelIndex>();
    EXPECT_EQ(changedIndex.row(), 1);
}
