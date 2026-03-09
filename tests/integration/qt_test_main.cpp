#include <gtest/gtest.h>

#include <QApplication>

/**
 * @brief Custom main for integration tests that require QApplication.
 *
 * SendingComponent creates Qt widgets (QWidget, QStackedWidget, etc.),
 * which require a QApplication instance to exist before construction.
 */
auto main(int argc, char** argv) -> int
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
