#include <gtest/gtest.h>

#include <QApplication>

/**
 * @brief Custom main for unit tests that require QApplication.
 *
 * This main initializes QApplication before running tests, which is required
 * for tests that create Qt widgets, use QSignalSpy, or rely on Qt's event loop.
 */
auto main(int argc, char** argv) -> int
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}