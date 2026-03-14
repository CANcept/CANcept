#include "monitoring_component.hpp"

#include "constants.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/macro/console_logging.hpp"
#include "dbc_file/dbc_component.hpp"
#include "monitoring/delegate/monitoring_delegate.hpp"
#include "monitoring/model/monitoring_model.hpp"
#include "monitoring/view/graph_list_view.hpp"
#include "monitoring/view/monitoring_view.hpp"

namespace Monitoring {

MonitoringComponent::MonitoringComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, Constants::MODULE_IDENTIFIER, Constants::TAB_TITLE,
                          QIcon(Constants::TAB_ICON_PATH)),
      m_model(std::make_unique<MonitoringModel>()),
      m_delegate(std::make_unique<MonitoringDelegate>(m_model.get())),
      m_view(std::make_unique<MonitoringView>(m_model.get(), m_delegate.get())),
      m_updateTimer(this)
{
    connectSignals();
}

MonitoringComponent::~MonitoringComponent()
{
    if (m_view && m_view->parent())
    {
        m_view.release();
    }
}

auto MonitoringComponent::getView() -> QWidget*
{
    return m_view.get();
}

void MonitoringComponent::connectSignals()
{
    connect(this, &MonitoringComponent::dbcConfigurationChanged, m_model.get(),
            &MonitoringModel::onDbcChange);
    connect(this, &MonitoringComponent::dbcConfigurationChanged, m_view->getSignalListView(),
            &SignalList::onDbcChange);
    connect(this, &MonitoringComponent::dbcConfigurationChanged, m_view->getGraphListView(),
            &GraphListView::onDbcChange);
    connect(this, &MonitoringComponent::dbcConfigurationChanged, this, [this]() {
        m_dbcLoaded = true;
        checkDeviceReadiness();
    });

    connect(this, &MonitoringComponent::dbcFrameReceived, m_model.get(),
            &MonitoringModel::onIncomingDbcFrame);

    connect(&m_updateTimer, &QTimer::timeout, m_view.get(), &MonitoringView::onUpdateMessages);
}

void MonitoringComponent::onStart()
{
    // subscribe to DBC Parsing successes
    m_parseSuccessConn = m_eventBroker.subscribe<Core::DBCParsedEvent>(
        [this](const Core::DBCParsedEvent& event) -> void {
            emit dbcConfigurationChanged(event.config);
        });

    // subscribe to incoming dbc decoded CAN traffic
    m_decodedFrameReceivedConn = m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
        [this](const Core::ReceivedCanDbcEvent& event) -> void {
            emit dbcFrameReceived(event.canMessage);
        });

    m_canDriverChangeConn = m_eventBroker.subscribe<Core::CanDriverChangeEvent>(
        [this](const Core::CanDriverChangeEvent&) {
            QMetaObject::invokeMethod(
                this, [this]() { checkDeviceReadiness(); }, Qt::QueuedConnection);
        });

    checkDeviceReadiness();
    m_updateTimer.start(Constants::REFRESH_INTERVAL_MS);
}

void MonitoringComponent::onStop()
{
    if (m_updateTimer.isActive())
    {
        m_updateTimer.stop();
    }
    m_parseSuccessConn = {};
    m_parseErrorConn = {};
    m_decodedFrameReceivedConn = {};
    m_canDriverChangeConn = {};
}

void MonitoringComponent::checkDeviceReadiness() const
{
    bool isReady = false;
    m_eventBroker.publish<Core::CheckCanDeviceReadyEvent>(Core::CheckCanDeviceReadyEvent(isReady));

    if (!isReady || !m_dbcLoaded)
    {
        m_view->showNoDbcOverlay();
        if (!isReady)
        {
            LOG_WRN("MonitoringComponent", "CAN device not configured");
        }
        if (!m_dbcLoaded)
        {
            LOG_INF("MonitoringComponent", "DBC file not loaded");
        }
    } else
    {
        m_view->hideNoDbcOverlay();
        LOG_INF("MonitoringComponent", "CAN device ready and DBC loaded");
    }
}

}  // namespace Monitoring
