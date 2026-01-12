#pragma once

#include <memory>
#include <vector>

#include "app_root/view/app_root_view.hpp"
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_lifecycle.hpp"
#include "core/interface/i_tab_component.hpp"
#include "tab_factory.hpp"

namespace AppRoot {

/**
 * @class AppRoot
 * @brief The central Kernel and Composition Root of the application.
 * * @details
 * This class encapsulates the entire application module. It follows a
 * "Self-Starting" pattern where bootstrap() triggers the internal lifecycle.
 */
class AppRoot
{
   public:
    /**
     * @brief Constructs the AppRoot kernel.
     */
    AppRoot();

    /**
     * @brief Triggers the internal shutdown sequence before destruction.
     * @details Ensures that the private shutdown() is called to release
     * hardware and save state before unique_ptrs clean up memory.
     */
    ~AppRoot();

    /**
     * @brief The sole entry point for the application module.
     * @details
     * 1. Performs Dependency Injection (Model/Delegate into View).
     * 2. Aggregates internal ITabComponents into the m_tabs vector.
     * 3. Calls the private start() method to initiate logic.
     */
    void bootstrap();

    // Deleted copy operations to ensure kernel uniqueness
    AppRoot(const AppRoot&) = delete;
    auto operator=(const AppRoot&) -> AppRoot& = delete;

   private:
    /**
     * @brief Initiates the internal logic and event loops.
     * @details Called automatically at the end of bootstrap().
     */
    void start();

    /**
     * @brief Gracefully stops internal components.
     * @details Called automatically by the destructor.
     */
    void shutdown();

    /**
     * @brief This function is called when a module had an Error and stopped.
     * @details It will try to restart the Tabs but prevents an infinite loop.
     * When the event broker or can handler fail the program will shut down.
     *
     * @param event the event of the module that had an Error
     */
    void restartModule(const Core::ModuleStoppedEvent& event);

    /**
     * @brief Registers a tab type in the factory and spawns the initial instance.
     * @tparam T Concrete class inheriting from ITabComponent.
     */
    template <typename T>
    void initTab()
    {
        m_tabFactory.registerCreator<T>([&]() -> auto { return std::make_unique<T>(*m_broker); });

        if (auto tab = m_tabFactory.create<T>())
        {
            m_model->addTab(tab.get());
            m_tabs.push_back(std::move(tab));
        }
    }

    /**
     * @brief A Connection to the QT Quit command.
     */
    QMetaObject::Connection m_qt_quit_connection;

    /**
     * @brief Pointer to the event broker of the application which handles the communication between
     * components.
     * @related Core::IEventBroker
     */
    std::unique_ptr<Core::IEventBroker> m_broker;
    std::unique_ptr<Core::ILifecycle> m_can_communication_handler;
    std::unique_ptr<Core::ILifecycle> m_dbc_handler;

    /**
     * @brief A tab factory which safes the initialization procedure of tabs.
     * @details This map maps the type of the tabs
     */
    TabFactory m_tabFactory;

    /**
     * @brief Handle for the ModuleStoppedEvent subscription.
     */
    Core::Connection m_module_stop_connection;

    /** * @brief Memory ownership of the tab components.
     * @details The Model only holds raw pointers; this vector ensures the objects
     * live as long as the AppRoot kernel.
     */
    std::vector<std::unique_ptr<Core::ITabComponent>> m_tabs;

    /**
     * @brief Pointer to the Model component of the app root to instantiate it.
     * @relationship "Instantiates" AppRootModel pointer.
     * @related AppRootModel
     */
    std::unique_ptr<AppRootModel> m_model;

    /**
     * @brief Pointer to the Delegate component of the app root to instantiate it.
     * @relationship "Instantiates" AppRootDelegate pointer.
     * @related AppRootDelegate
     */
    std::unique_ptr<AppRootDelegate> m_delegate;

    /**
     * @brief Pointer to the View component of the app root to instantiate it.
     * @relationship "Instantiates" AppRootView pointer.
     * @related AppRootView
     */
    std::unique_ptr<AppRootView> m_mainView;
};

}  // namespace AppRoot