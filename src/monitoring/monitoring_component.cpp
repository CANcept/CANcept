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

// Constructor / destructor
MonitoringComponent::MonitoringComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, Constants::MODULE_IDENTIFIER, Constants::TAB_TITLE,
                          QIcon(Constants::TAB_ICON_PATH)),
      m_model(std::make_unique<MonitoringModel>()),
      m_delegate(std::make_unique<MonitoringDelegate>(m_model.get())),
      m_view(std::make_unique<MonitoringView>(m_model.get(), m_delegate.get()))
{
    // --- Internal Signal/Slot Connections ---

    // Connect View UI actions to this Component's logic
    connect(m_view.get(), &MonitoringView::signalChecked, this,
            &MonitoringComponent::onSignalChecked);

    connect(m_view.get(), &MonitoringView::signalUnchecked, this,
            &MonitoringComponent::onSignalUnchecked);

    // Forward internal signal to the view to refresh UI (e.g. the TreeView)
    connect(this, &MonitoringComponent::dbcConfigurationChanged, m_view.get(),
            &MonitoringView::onDbcConfigurationChanged);

    // Forward dbc decoded frame received signal to the model
    connect(this, &MonitoringComponent::dbcFrameReceived, m_model.get(),
            &MonitoringModel::onIncomingDbcFrame);

    // Forward raw frame received signal to the model
    connect(this, &MonitoringComponent::rawFrameReceived, m_model.get(),
            &MonitoringModel::onIncomingRawFrame);

    // Device selection changes
    connect(m_view->interfaceSelector(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) -> void {
                m_interfaceSelected = (index >= 0);
                if (index >= 0)
                {
                    const auto deviceName =
                        m_view->interfaceSelector()->currentText().toStdString();
                    onDeviceChanged(deviceName);
                }
            });

    // Mode selection changes
    connect(m_view->modeToggle(), &QPushButton::clicked, this,
            [this]() -> void { m_dbcModeEnabled = !m_dbcModeEnabled; });
}

MonitoringComponent::~MonitoringComponent() = default;

// Return a QWidget pointer
auto MonitoringComponent::getView() -> QWidget*
{
    return m_view.get();
}

void MonitoringComponent::onStart()
{
    // Subscribe to incoming raw CAN traffic
    m_rawFrameReceivedConn = m_eventBroker.subscribe<Core::ReceivedCanRawEvent>(
        [this](const Core::ReceivedCanRawEvent& event) -> void {
            // We emit a signal so the View/Delegate can update the UI safely
            // on the main thread if the broker operates on a background thread.
            emit rawFrameReceived(event.canMessage);
        });

    // 2. Subscribe to DBC Parsing successes
    m_parseSuccessConn = m_eventBroker.subscribe<Core::DBCParsedEvent>(
        [this](const Core::DBCParsedEvent& event) -> void {
            // Logic to update model with new DBC data could go here
            emit dbcConfigurationChanged();
        });

    // 3. Subscribe to incoming dbc decoded CAN traffic
    m_decodedFrameReceivedConn = m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
        [this](const Core::ReceivedCanDbcEvent& event) -> void {
            // We emit a signal so the View/Delegate can update the UI safely
            // on the main thread if the broker operates on a background thread.
            emit dbcFrameReceived(event.canMessage);
        });
}
void MonitoringComponent::onStop()
{
    // Disconnect event broker handles to prevent callbacks to a stopped component
    m_parseSuccessConn = {};
    m_parseErrorConn = {};
}

void MonitoringComponent::onDeviceChanged(const std::string& deviceName) const
{
    LOG_INF("MonitoringComponent", "CAN device changed to: {}", deviceName);
    m_eventBroker.publish<Core::CanDriverChangeEvent>(Core::CanDriverChangeEvent(deviceName));
}

// Slots

void MonitoringComponent::onSignalChecked(char messageId, const std::string& signalName)
{
    // Delegate handles the logic of adding the signal to the internal graph list
    if (m_view)
    {
        m_view->getGraphListView()->addGraph(messageId, signalName);
    }
}

void MonitoringComponent::onSignalUnchecked(char messageId, const std::string& signalName)
{
    // Delegate handles removal and cleanup of the graph
    if (m_view)
    {
        m_view->getGraphListView()->deleteGraph(messageId, signalName);
    }
}
}  // namespace Monitoring
