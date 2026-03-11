#include <gtest/gtest.h>

#include <QApplication>

/**
 * @brief Custom main for system tests that require QApplication.
 *
 * AppRoot creates Qt widgets and a main window, which require a QApplication
 * instance to exist before any widget construction.
 */
auto main(int argc, char** argv) -> int
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
