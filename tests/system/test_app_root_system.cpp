#include <gtest/gtest.h>
#include <unistd.h>

#include <QApplication>
#include <QComboBox>
#include <QListView>
#include <QPalette>
#include <QPushButton>
#include <QStackedWidget>
#include <QTest>

#include "app_root/constants.hpp"
#include "app_root/entry_point/app_root.hpp"
#include "app_root/view/app_root_view.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"

class AppRootSystemTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        if (getuid() != 0)
        {
            GTEST_SKIP() << "System tests require root privileges (run start.sh with sudo)";
        }

        deviceManager = std::make_unique<TestHelpers::SocketCanDeviceManager>("vcan0");
        try
        {
            deviceManager->create();
            deviceManager->up();
            vcanCreated = true;
        } catch (const std::exception& e)
        {
            GTEST_SKIP() << "Failed to set up vcan0: " << e.what();
        }
    }

    void TearDown() override
    {
        if (vcanCreated)
        {
            deviceManager->down();
            deviceManager->remove();
        }
    }

    auto findAppRootView() const -> AppRoot::AppRootView*
    {
        for (QWidget* widget : QApplication::topLevelWidgets())
        {
            if (auto* view = qobject_cast<AppRoot::AppRootView*>(widget))
            {
                return view;
            }
        }
        return nullptr;
    }

    void openSettings(const AppRoot::AppRootView* view) const
    {
        auto* btn = view->findChild<QPushButton*>(QString(), Qt::FindDirectChildrenOnly);
        ASSERT_NE(btn, nullptr) << "Settings button not found";
        btn->click();
        QTest::qWait(50);
    }

    auto findComboWithItem(const QWidget* root, const QString& itemText) const -> QComboBox*
    {
        for (auto* combo : root->findChildren<QComboBox*>())
        {
            for (int i = 0; i < combo->count(); ++i)
            {
                if (combo->itemText(i) == itemText)
                {
                    return combo;
                }
            }
        }
        return nullptr;
    }

    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    bool vcanCreated = false;
};

TEST_F(AppRootSystemTest, Bootstrap_RegistersAllFourTabs)
{
    AppRoot::AppRoot appRoot;
    appRoot.bootstrap();
    QTest::qWait(100);

    auto* appRootView = findAppRootView();
    ASSERT_NE(appRootView, nullptr) << "AppRootView not found among top-level widgets";

    const auto* listView = appRootView->findChild<QListView*>();
    ASSERT_NE(listView, nullptr) << "Tab QListView not found inside AppRootView";
    ASSERT_NE(listView->model(), nullptr);
    EXPECT_EQ(listView->model()->rowCount(), 4)
        << "Expected 4 tabs: DBC, Monitoring, Sending, Logging";
}

TEST_F(AppRootSystemTest, Bootstrap_InitialThemeSetting_MatchesSystemPalette)
{
    AppRoot::AppRoot appRoot;
    appRoot.bootstrap();
    QTest::qWait(100);

    auto* appRootView = findAppRootView();
    ASSERT_NE(appRootView, nullptr);

    openSettings(appRootView);

    auto* themeCombo =
        findComboWithItem(appRootView, QString::fromStdString(AppRoot::Constants::THEME_LIGHT));
    ASSERT_NE(themeCombo, nullptr) << "Theme combo box not found in settings";

    const QPalette sysPalette = QApplication::palette();
    const QString expectedTheme = (sysPalette.color(QPalette::Window).lightness() <
                                   sysPalette.color(QPalette::WindowText).lightness())
                                      ? QString::fromStdString(AppRoot::Constants::THEME_DARK)
                                      : QString::fromStdString(AppRoot::Constants::THEME_LIGHT);

    EXPECT_EQ(themeCombo->currentText(), expectedTheme)
        << "Theme combo should be pre-selected to match system palette preference";
}

TEST_F(AppRootSystemTest, Bootstrap_CanInterfaceSetting_SelectingVcan_UpdatesCombo)
{
    AppRoot::AppRoot appRoot;
    appRoot.bootstrap();
    QTest::qWait(100);

    auto* appRootView = findAppRootView();
    ASSERT_NE(appRootView, nullptr);

    openSettings(appRootView);

    auto* interfaceCombo = findComboWithItem(appRootView, "vcan0");
    ASSERT_NE(interfaceCombo, nullptr)
        << "\"vcan0\" not found in interface combo options — device not detected";

    interfaceCombo->setCurrentText("vcan0");
    QTest::qWait(100);

    EXPECT_EQ(interfaceCombo->currentText(), "vcan0")
        << "Interface combo should reflect the selected vcan0 device";
}
