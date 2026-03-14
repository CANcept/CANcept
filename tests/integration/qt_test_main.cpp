#include <gtest/gtest.h>

#include <QApplication>

/**
 * @brief Custom main for integration tests that require QApplication.
 */
auto main(int argc, char** argv) -> int
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
