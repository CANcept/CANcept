/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include "core/theme/color_themes.hpp"
#include "core/theme/theme_manager.hpp"
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

TEST_F(AppRootSystemTest, ThemeChange_AllThemes_ApplyCorrectColorTheme)
{
    AppRoot::AppRoot appRoot;
    appRoot.bootstrap();
    QTest::qWait(100);

    auto* appRootView = findAppRootView();
    ASSERT_NE(appRootView, nullptr);

    openSettings(appRootView);

    const struct {
        std::string name;
        QColor expectedSurfaceMain;
    } cases[] = {
        {AppRoot::Constants::THEME_DARK, QColor(0x1e, 0x1e, 0x2e)},
        {AppRoot::Constants::THEME_AQUA, QColor(0xf0, 0xf4, 0xf8)},
        {AppRoot::Constants::THEME_MAROON, QColor(0xf8, 0xf4, 0xed)},
        {AppRoot::Constants::THEME_DRACULA, QColor(0x28, 0x29, 0x36)},
        {AppRoot::Constants::THEME_LIGHT, QColor(0xff, 0xff, 0xff)},
    };

    for (const auto& [name, expectedSurfaceMain] : cases)
    {
        // Re-fetch each iteration: SettingsView::rebuild() recreates widgets on StyleEvent
        auto* combo =
            findComboWithItem(appRootView, QString::fromStdString(AppRoot::Constants::THEME_LIGHT));
        ASSERT_NE(combo, nullptr) << "Theme combo not found for: " << name;

        const int idx = combo->findText(QString::fromStdString(name));
        ASSERT_GE(idx, 0) << "Theme not found in combo: " << name;

        combo->setCurrentIndex(idx);
        QTest::qWait(0);  // drain: StyleEvents → singleShot(0) scheduled → fires → rebuild()

        EXPECT_EQ(Core::ThemeManager::getInstance().colors().surfaceMain, expectedSurfaceMain)
            << "Wrong surfaceMain for theme: " << name;
    }
}
