#include <gtest/gtest.h>
#include <memory>
#include <QList>

#include "dbc_file/model/dbc_item.hpp"

using namespace DbcFile;

class DbcItemTest : public ::testing::Test {
protected:
    // Helper to quickly create an item with a name and type
    std::unique_ptr<DbcItem> createItem(const QString& name, Core::DbcItemType type = Core::DbcItemType::Ecu) {
        return std::make_unique<DbcItem>(QList<QVariant>{name}, type);
    }
};

TEST_F(DbcItemTest, InitialState_IsCorrect) {
    QList<QVariant> data = {"MyItem", 42};
    auto item = std::make_unique<DbcItem>(data, Core::DbcItemType::Message);

    // Verify constructor assignments
    EXPECT_EQ(item->type(), Core::DbcItemType::Message);
    EXPECT_EQ(item->columnCount(), 2);
    EXPECT_EQ(item->data(0).toString(), "MyItem");
    EXPECT_EQ(item->data(1).toInt(), 42);

    // Verify default state for relationships
    EXPECT_EQ(item->childCount(), 0);
    EXPECT_EQ(item->parent(), nullptr);
}

TEST_F(DbcItemTest, SetData_ResizesVector) {
    auto item = createItem("Test");

    // Initial size is 1 column
    EXPECT_EQ(item->columnCount(), 1);

    // 1. Update existing column (no resize needed)
    item->setData(0, "Updated");
    EXPECT_EQ(item->data(0).toString(), "Updated");

    // 2. Trigger Auto-Resize: Set data at index 5
    // Logic: if (column >= m_data.size()) m_data.resize(column + 1);
    item->setData(5, "FarAway");

    // Size should now be 6 (indices 0 to 5)
    EXPECT_EQ(item->columnCount(), 6);
    EXPECT_EQ(item->data(5).toString(), "FarAway");

    // Intermediate indices (1-4) should be invalid/empty QVariants
    EXPECT_FALSE(item->data(1).isValid());
}

TEST_F(DbcItemTest, SetData_IgnoresNegativeIndex) {
    auto item = createItem("Test");

    // Logic: if (column < 0) return;
    item->setData(-1, "Boom");

    // Should not crash and not change size
    EXPECT_EQ(item->columnCount(), 1);
}

TEST_F(DbcItemTest, Data_BoundaryChecks) {
    auto item = createItem("Test");

    // Logic: if (column < 0 || column >= m_data.size()) return QVariant();

    // 1. Negative index
    EXPECT_FALSE(item->data(-1).isValid());

    // 2. Out of bounds index
    EXPECT_FALSE(item->data(99).isValid());
}

TEST_F(DbcItemTest, AppendChild_SetsParentAndOrder) {
    auto root = createItem("Root");

    // Create and append first child
    auto child1 = createItem("Child1");
    DbcItem* ptr1 = child1.get(); // Store raw pointer for verification
    root->appendChild(std::move(child1));

    // Verify linkage
    ASSERT_EQ(root->childCount(), 1);
    EXPECT_EQ(root->child(0), ptr1);
    EXPECT_EQ(ptr1->parent(), root.get()); // Verify m_parent assignment

    // Create and append second child
    auto child2 = createItem("Child2");
    DbcItem* ptr2 = child2.get();
    root->appendChild(std::move(child2));

    // Verify order (FIFO)
    ASSERT_EQ(root->childCount(), 2);
    EXPECT_EQ(root->child(1), ptr2);
    EXPECT_EQ(ptr2->parent(), root.get());
}

TEST_F(DbcItemTest, PrependChild_InsertsAtFront) {
    auto root = createItem("Root");

    // Add an initial child
    root->appendChild(createItem("OldFirst"));

    // Prepend a new child
    auto newChild = createItem("NewFirst");
    DbcItem* ptrNew = newChild.get();
    root->prependChild(std::move(newChild));

    ASSERT_EQ(root->childCount(), 2);

    // The new child must be at index 0 now
    EXPECT_EQ(root->child(0), ptrNew);
    EXPECT_EQ(root->child(0)->data(0).toString(), "NewFirst");

    // The old child moved to index 1
    EXPECT_EQ(root->child(1)->data(0).toString(), "OldFirst");

    // Verify parent linkage
    EXPECT_EQ(ptrNew->parent(), root.get());
}

TEST_F(DbcItemTest, Row_CalculatesIndexInParent) {
    auto root = createItem("Root");

    // Add 3 children
    auto child1 = createItem("C1");
    DbcItem* ptr1 = child1.get();
    root->appendChild(std::move(child1));

    auto child2 = createItem("C2");
    DbcItem* ptr2 = child2.get();
    root->appendChild(std::move(child2));

    auto child3 = createItem("C3");
    DbcItem* ptr3 = child3.get();
    root->appendChild(std::move(child3));

    // Logic: Loop through parent's children to find self
    EXPECT_EQ(ptr1->row(), 0);
    EXPECT_EQ(ptr2->row(), 1);
    EXPECT_EQ(ptr3->row(), 2);
}
TEST_F(DbcItemTest, Row_ReturnsZero_IfItemNotFoundInParent) {
    auto parent = createItem("Parent");
    parent->appendChild(createItem("RealChild")); // Index 0

    auto stranger = createItem("Stranger");

    stranger->setParent(parent.get());
    EXPECT_EQ(stranger->row(), 0);
}

TEST_F(DbcItemTest, Row_ReturnsZeroWithoutParent) {
    auto orphan = createItem("Orphan");

    // Logic: if (m_parent) ... else return 0;
    // Item has no parent set
    EXPECT_EQ(orphan->row(), 0);
}

TEST_F(DbcItemTest, Child_BoundaryChecks) {
    auto root = createItem("Root");
    root->appendChild(createItem("Child"));

    // Logic: if (m_children.empty() || row >= size || row < 0) return nullptr;

    // 1. Valid access
    EXPECT_NE(root->child(0), nullptr);

    // 2. Out of bounds (too high)
    EXPECT_EQ(root->child(1), nullptr);

    // 3. Out of bounds (negative)
    EXPECT_EQ(root->child(-1), nullptr);

    // 4. Access on empty item
    auto emptyItem = createItem("Empty");
    EXPECT_EQ(emptyItem->child(0), nullptr);
}

TEST_F(DbcItemTest, GetChildren_ReturnsConstReference) {
    auto root = createItem("Root");
    root->appendChild(createItem("C1"));

    // Verify we get access to the underlying vector
    const auto& children = root->getChildren();
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children[0]->data(0).toString(), "C1");
}

TEST_F(DbcItemTest, SetParent_ExplicitCall) {
    auto child = createItem("Child");
    auto dad = createItem("Dad");

    // Manually setting parent (rarely used but part of API)
    child->setParent(dad.get());
    EXPECT_EQ(child->parent(), dad.get());
}