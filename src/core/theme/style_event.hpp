#pragma once
#include <QEvent>

namespace Core {

/**
 * @brief A custom QT event send when the Theme changes.
 */
class StyleEvent final : public QEvent
{
   public:
    static const QEvent::Type EventType;

    StyleEvent() : QEvent(EventType) {}
};

}  // namespace Core