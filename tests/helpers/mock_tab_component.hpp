#pragma once

#include <QIcon>
#include <QString>

#include "core/interface/i_tab_component.hpp"

namespace TestHelpers {

/**
 * @brief Mock implementation of ITabComponent for testing.
 */
class MockTabComponent : public Core::ITabComponent
{
   public:
    explicit MockTabComponent(Core::IEventBroker& broker, QString id = "mock_tab",
                              QString title = "Mock Tab", QIcon icon = QIcon())
        : ITabComponent(broker, std::move(id), std::move(title), std::move(icon))
    {
    }

    ~MockTabComponent() override = default;

    void onStart() override {}
    void onStop() override {}
    auto getView() -> QWidget* override
    {
        return nullptr;
    }
};
}  // namespace TestHelpers
