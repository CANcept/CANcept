#include <gtest/gtest.h>

#include <QApplication>
#include <QtGlobal>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(resources);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}