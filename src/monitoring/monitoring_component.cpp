#include "monitoring_component.hpp"

namespace Monitoring {

// Constructor / destructor
MonitoringComponent::MonitoringComponent(Core::IEventBroker& broker)
    : ITabComponent(broker, "stub", "stub"), m_model(nullptr), m_view(nullptr), m_delegate(nullptr)
{
}

MonitoringComponent::~MonitoringComponent() = default;

// Return a dummy QWidget pointer (nullptr for now)
auto MonitoringComponent::getView() -> QWidget*
{
    return nullptr;
}

// Empty lifecycle methods
void MonitoringComponent::onStart() {}
void MonitoringComponent::onStop() {}

// Dummy slots
void MonitoringComponent::onSourceChanged(const std::string&) {}
void MonitoringComponent::onSignalChecked(char, const std::string&) {}
void MonitoringComponent::onSignalUnchecked(char, const std::string&) {}

}  // namespace Monitoring
