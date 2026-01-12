#include "app_root/entry_point/app_root.hpp"

#include <qcoreapplication.h>

#include "app_root/model/app_root_model.hpp"
#include "app_root/view/app_root_view.hpp"
#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "core/macro/console_logging.hpp"
#include "dbc_file/dbc_component.hpp"
#include "event_broker/event_broker.hpp"
#include "logging/logging_component.hpp"
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

    LOG_INF("AppRoot", "Instantiating Event Broker...");
    m_broker = std::make_unique<EventBroker::EventBroker>();

    LOG_INF("AppRoot", "Instantiating Can Communication Handler...");
    m_can_communication_handler = std::make_unique<CanHandler::CanCommunicationHandler>(*m_broker);

    LOG_INF("AppRoot", "Instanciating Dbc Handler");
    m_dbc_handler = std::make_unique<CanHandler::DbcHandler>(*m_broker);

    LOG_INF("AppRoot", "Instantiating App Root MVD...");
    m_model = std::make_unique<AppRootModel>();
    m_delegate = std::make_unique<AppRootDelegate>();
    m_mainView = std::make_unique<AppRootView>();

    //  Wiring Model, Delegate nad View of App Root
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

    // Helper to keep bootstrap readable
    initTab<DbcFile::DbcComponent>();
    initTab<Monitoring::MonitoringComponent>();
    initTab<Sending::SendingComponent>();
    initTab<Logging::LoggingComponent>();

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
    static bool shuttingDown = false;
    if (shuttingDown) return;
    shuttingDown = true;

    // disconnect to not be called by qt again
    QObject::disconnect(m_qt_quit_connection);

    LOG_INF("AppRoot", "Shutting down...");

    if (m_module_stop_connection) m_module_stop_connection.release();

    if (m_broker) m_broker->publish<Core::AppStoppedEvent>({});

    m_tabs.clear();
    m_mainView.reset();
    m_delegate.reset();
    m_model.reset();
    m_can_communication_handler.reset();
    m_dbc_handler.reset();
    m_broker.reset();
}

// Invariant:
// - canRestart() == true only for restartable tabs
// - Infrastructure modules (broker, CAN) are fatal
void AppRoot::restartModule(const Core::ModuleStoppedEvent& event)
{
    const auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [&](const auto& t) -> auto {
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

    // Restart new tab
    if (auto newTab = m_tabFactory.createByTypeIndex(event.module_index))
    {
        m_model->replaceTab(it->get(), newTab.get());
        *it = std::move(newTab);

        LOG_INF("AppRoot", "Restarted: {}", event.module_index.name());
    }
}

}  // namespace AppRoot