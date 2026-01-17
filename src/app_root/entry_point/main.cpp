#include <QApplication>

#include "app_root.hpp"

/**
 * @brief Application Entry Point.
 * @details Initializes the Qt environment and hands control to the AppRoot kernel.
 */
auto main(int argc, char* argv[]) -> int
{
    QApplication app(argc, argv);
    AppRoot::AppRoot kernel;
    kernel.bootstrap();
    return app.exec();
}