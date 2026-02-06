#include "style_event.hpp"

#include <QApplication>
#include <QWidget>

namespace Core {

// Register custom event type
const QEvent::Type StyleEvent::EventType = static_cast<QEvent::Type>(QEvent::registerEventType());

}  // namespace Core