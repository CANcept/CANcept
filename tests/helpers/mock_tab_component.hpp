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

#pragma once

#include <QIcon>
#include <QLabel>
#include <QPointer>
#include <QString>

#include "core/interface/i_tab_component.hpp"

namespace TestHelpers {

/**
 * @brief Mock ITabComponent that returns nullptr from getView(). Use for model/data tests.
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

/**
 * @brief ITabComponent that counts onStart() / onStop() calls.
 */
class LifecycleTrackingTabComponent final : public Core::ITabComponent
{
   public:
    explicit LifecycleTrackingTabComponent(Core::IEventBroker& broker, QString id = "tracking_tab",
                                           QString title = "Tracking Tab")
        : ITabComponent(broker, std::move(id), std::move(title))
    {
    }

    ~LifecycleTrackingTabComponent() override = default;

    void onStart() override
    {
        ++startCount;
    }
    void onStop() override
    {
        ++stopCount;
    }
    auto getView() -> QWidget* override
    {
        return nullptr;
    }

    int startCount{0};
    int stopCount{0};
};

/**
 * @brief ITabComponent backed by a real QLabel widget.
 */
class RealWidgetTabComponent final : public Core::ITabComponent
{
   public:
    explicit RealWidgetTabComponent(Core::IEventBroker& broker, QString id = "real_tab",
                                    QString title = "Real Tab")
        : ITabComponent(broker, std::move(id), std::move(title)), m_widget(new QLabel(getId()))
    {
    }

    ~RealWidgetTabComponent() override
    {
        if (m_widget)
        {
            delete m_widget;
        }
    }

    void onStart() override {}
    void onStop() override {}
    auto getView() -> QWidget* override
    {
        return m_widget;
    }

   private:
    QPointer<QLabel> m_widget;
};

}  // namespace TestHelpers
