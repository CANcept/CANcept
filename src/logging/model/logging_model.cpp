#include "logging_model.hpp"

#include "core/util/log_service.hpp"

namespace Logging {
void LoggingModel::onRawFrameReceived(const Core::RawCanMessage& msg)
{
    // This static call creates a dependency relationship
    auto logger = Core::LogService::getInstance().getLogger(
        Core::LogContext::CanLogging, m_sessions[m_activeSessionIndex].id.toStdString());
}
}  // namespace Logging
