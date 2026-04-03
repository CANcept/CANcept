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

#include "app_root/entry_point/app_root.hpp"

#include <qcoreapplication.h>
#include <qfile.h>

#include <QApplication>
#include <QPalette>
#include <algorithm>

#include "app_root/constants.hpp"
#include "app_root/model/app_root_model.hpp"
#include "app_root/view/app_root_view.hpp"
#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "core/dto/setting_dto.hpp"
#include "core/event/theme_event.hpp"
#include "core/macro/console_logging.hpp"
#include "core/theme/color_themes.hpp"
#include "core/theme/spacing_themes.hpp"
#include "core/theme/theme_manager.hpp"
#include "dbc_file/dbc_component.hpp"
#include "event_broker/event_broker.hpp"
#include "logging/logging_component.hpp"
#include "math/service/variable_registry.hpp"
#include "monitoring/monitoring_component.hpp"
#include "sending/sending_component.hpp"

namespace AppRoot {

AppRoot::AppRoot() = default;

AppRoot::~AppRoot()
{
    // The kernel should shut down all dependencies in the correct order first.
    shutdown();
}

void AppRoot::bootstrap()
{
    LOG_INF("AppRoot", "Starting bootstrap...");

    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    qApp->setStyleSheet(QString("QToolTip {"
                                "background-color: %1;"
                                "color: %2;"
                                "border: %3px solid %4;"
                                "border-radius: %5px;"
                                "opacity: 250;"
                                "}")
                            .arg(colors.surfaceMain.name(), colors.textSecondary.name())
                            .arg(spacing.borderThin)
                            .arg(colors.borderStrong.name(QColor::HexArgb))
                            .arg(spacing.radiusXs));

    LOG_INF("AppRoot", "Loading theme...");
    Core::ThemeManager::getInstance().setColorTheme(std::make_unique<Core::LightTheme>());
    Core::ThemeManager::getInstance().setSpacingTheme(std::make_unique<Core::NormalSpacingTheme>());

    LOG_INF("AppRoot", "Instantiating Event Broker...");
    m_broker = std::make_unique<EventBroker::EventBroker>();

    LOG_INF("AppRoot", "Instantiating Settings Service...");
    m_settingsService = std::make_unique<SettingsService>(*m_broker);

    LOG_INF("AppRoot", "Registering app root settings...");
    m_settingsService->registerSetting(
        std::make_unique<Core::SettingDefinition<
            Core::SettingType::Select, Core::GetAvailableThemesEvent, Core::ThemeChangeEvent>>(
            Core::SettingKey{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID},
            Constants::THEME_ICON_PATH,
            Core::TypeTraits<Core::SettingType::Select, Core::GetAvailableThemesEvent>{
                "Select Theme"}));

    // Detect system theme preference and set initial theme value
    LOG_INF("AppRoot", "Detecting system theme preference...");
    std::string initialTheme = Constants::THEME_LIGHT;
    const QPalette systemPalette = QApplication::palette();
    const QColor windowColor = systemPalette.color(QPalette::Window);
    const QColor textColor = systemPalette.color(QPalette::WindowText);

    if (windowColor.lightness() < textColor.lightness())
    {
        initialTheme = Constants::THEME_DARK;
        LOG_INF("AppRoot", "System dark mode detected, setting initial theme to Dark");
        Core::ThemeManager::getInstance().setColorTheme(std::make_unique<Core::DarkTheme>());
    } else
    {
        LOG_INF("AppRoot", "System light mode detected, keeping initial theme as Light");
    }

    m_settingsService->setValue(
        Core::SettingKey{Constants::THEME_SETTING_ID, Constants::THEME_COMPONENT_ID}, initialTheme);

    LOG_INF("AppRoot", "Instantiating Can Communication Handler...");
    m_can_communication_handler = std::make_unique<CanHandler::CanCommunicationHandler>(*m_broker);
    m_can_communication_handler->registerSettings(*m_settingsService);

    LOG_INF("AppRoot", "Instanciating Dbc Handler");
    m_dbc_handler = std::make_unique<CanHandler::DbcHandler>(*m_broker);
    m_dbc_handler->registerSettings(*m_settingsService);

    LOG_INF("AppRoot", "Instantiating Variable Registry...");
    m_variableRegistry = std::make_unique<Math::VariableRegistry>(*m_broker);

    LOG_INF("AppRoot", "Instantiating App Root MVD...");
    m_model = std::make_unique<AppRootModel>();
    m_settingsModel = std::make_unique<SettingsModel>(*m_settingsService, *m_broker);
    m_delegate = std::make_unique<AppRootDelegate>();
    m_mainView = std::make_unique<AppRootView>();

    if (m_mainView && m_model)
    {
        m_mainView->setModel(m_model.get());
    }

    if (m_mainView && m_delegate)
    {
        m_mainView->setDelegate(m_delegate.get());
    }

    LOG_INF("AppRoot", "Adding and Instatiating Tabs...");
    m_tabs.clear();

    initTab<DbcFile::DbcComponent>();
    initTab<Monitoring::MonitoringComponent>();
    initTab<Sending::SendingComponent>();
    initTab<Logging::LoggingComponent>();

    // Inject variable registry into sending component
    for (const auto& tab : m_tabs)
    {
        if (auto* sending = dynamic_cast<Sending::SendingComponent*>(tab.get()))
        {
            sending->setVariableRegistry(
                dynamic_cast<Math::VariableRegistry*>(m_variableRegistry.get()));
        }
    }

    for (const auto& tab : m_tabs)
    {
        tab->registerSettings(*m_settingsService);
    }

    if (m_mainView && m_settingsModel)
    {
        m_mainView->setSettingsModel(m_settingsModel.get());
    }

    LOG_INF("AppRoot", "Bootstrap Complete: launching internal logic.");
    start();
}

void AppRoot::start()
{
    // Connect shutdown to QT Core for window close etc.
    m_qt_quit_connection =
        QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                         [this]() -> void { shutdown(); });

    // Tabs Restart on Error while the Broker and Can Handler are fatal
    m_module_stop_connection = m_broker->subscribe<Core::ModuleStoppedEvent>(
        [this](const Core::ModuleStoppedEvent& event) -> void {
            if (event.diagnostics.wasError)
            {
                restartModule(event);
            }
        });

    // Provide available theme options when the settings UI requests them
    m_themeProviderConn = m_broker->subscribe<Core::GetAvailableThemesEvent>(
        [](const Core::GetAvailableThemesEvent& event) {
            event.options->push_back(
                Core::SelectOption{Constants::THEME_LIGHT, Constants::THEME_LIGHT});
            event.options->push_back(
                Core::SelectOption{Constants::THEME_DARK, Constants::THEME_DARK});
            event.options->push_back(
                Core::SelectOption{Constants::THEME_AQUA, Constants::THEME_AQUA});
            event.options->push_back(
                Core::SelectOption{Constants::THEME_MAROON, Constants::THEME_MAROON});
            event.options->push_back(
                Core::SelectOption{Constants::THEME_DRACULA, Constants::THEME_DRACULA});
        });

    // Apply the selected theme when the user changes it
    m_themeChangeConn = m_broker->subscribe<Core::ThemeChangeEvent>([](const Core::ThemeChangeEvent&
                                                                           event) {
        if (event.themeName == Constants::THEME_DARK)
        {
            Core::ThemeManager::getInstance().setColorTheme(std::make_unique<Core::DarkTheme>());
        } else if (event.themeName == Constants::THEME_AQUA)
        {
            Core::ThemeManager::getInstance().setColorTheme(std::make_unique<Core::AquaTheme>());
        } else if (event.themeName == Constants::THEME_MAROON)
        {
            Core::ThemeManager::getInstance().setColorTheme(std::make_unique<Core::MaroonTheme>());
        } else if (event.themeName == Constants::THEME_DRACULA)
        {
            Core::ThemeManager::getInstance().setColorTheme(std::make_unique<Core::DraculaTheme>());
        } else
        {
            Core::ThemeManager::getInstance().setColorTheme(std::make_unique<Core::LightTheme>());
        }
    });

    LOG_INF("AppRoot", "Application started: publishing AppStartedEvent!");
    m_broker->publish<Core::AppStartedEvent>(Core::AppStartedEvent());

    if (m_mainView)
    {
        m_mainView->show();
    }
}

void AppRoot::shutdown()
{
    // to prevent being called twice
    if (m_shuttingDown) return;
    m_shuttingDown = true;

    // disconnect to not be called by qt again
    QObject::disconnect(m_qt_quit_connection);

    LOG_INF("AppRoot", "Shutting down...");

    if (m_module_stop_connection) m_module_stop_connection.release();
    if (m_themeProviderConn) m_themeProviderConn.release();
    if (m_themeChangeConn) m_themeChangeConn.release();

    if (m_broker) m_broker->publish<Core::AppStoppedEvent>({});

    m_tabs.clear();
    m_mainView.reset();
    m_delegate.reset();
    m_settingsModel.reset();
    m_model.reset();
    m_variableRegistry.reset();
    m_can_communication_handler.reset();
    m_dbc_handler.reset();
    m_settingsService.reset();
    m_broker.reset();
}

// Invariant:
// - canRestart() == true only for restartable tabs
// - Infrastructure modules (broker, CAN) are fatal
void AppRoot::restartModule(const Core::ModuleStoppedEvent& event)
{
    const auto it = std::ranges::find_if(m_tabs, [&](const auto& t) -> auto {
        return std::type_index(typeid(*t)) == event.module_index;
    });

    // Event Broker and Can Handler are Fatal
    if (it == m_tabs.end() || !m_tabFactory.canRestart(event.module_index))
    {
        LOG_ERR("AppRoot",
                "Received stop event for type {} which can't be restarted. Shutting down.",
                event.module_index.name());
        QMetaObject::invokeMethod(
            QCoreApplication::instance(), []() -> void { QCoreApplication::exit(EXIT_FAILURE); },
            Qt::QueuedConnection);
        return;
    }

    LOG_ERR("AppRoot", "Received stop event for Tab: {}. Attempting restart",
            event.module_index.name());

    if (auto newTab = m_tabFactory.createByTypeIndex(event.module_index))
    {
        newTab->registerSettings(*m_settingsService);
        m_model->replaceTab(it->get(), newTab.get());
        *it = std::move(newTab);

        LOG_INF("AppRoot", "Restarted: {}", event.module_index.name());
    }
}

}  // namespace AppRoot