#pragma once

#ifndef __cpp_constinit
#define QT_CONSTINIT
#endif

#include <QIcon>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QWidget>

#include "i_lifecycle.hpp"

namespace Core {

/**
 * @brief Interface for a UI component that represents a single tab in the application.
 *
 * This class combines the application lifecycle (Start/Stop) with the UI requirements
 * needed to render a Tab inside our Qt based GUI. It inherits from QObject to support
 * signaling UI changes like title or icon updates.
 */
class ITabComponent : public QObject, public ILifecycle
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the tab component.
     * @param broker Reference to the event broker for communication.
     * @param id A unique internal identifier for this tab instance.
     * @param title The display name of the tab in the UI.
     * @param icon The icon displayed next to the Title.
     */
    ITabComponent(IEventBroker& broker, QString id, QString title, QIcon icon = QIcon())
        : ILifecycle(broker),
          m_id(std::move(id)),
          m_title(std::move(title)),
          m_icon(std::move(icon))
    {
    }

    /**
     * @brief Virtual destructor.
     */
    virtual ~ITabComponent() override;

    /**
     * @brief Returns the Qt widget associated with this tab.
     * @details The App Root will call this to retrieve the
     * visual representation of the Tab for embedding into the GUI.
     * @return QWidget* Pointer to the tab's root widget.
     */
    virtual auto getView() -> QWidget* = 0;

    /**
     * @brief Returns the icon associated with this tab.
     * @return QIcon The tab icon.
     */
    [[nodiscard]] virtual auto getIcon() const -> QIcon
    {
        return m_icon;
    }

    /**
     * @brief Returns the display title of the tab.
     * @return const QString& The title string.
     */
    [[nodiscard]] auto getTitle() const -> const QString&
    {
        return m_title;
    }

    /**
     * @brief Returns the unique identifier of the tab.
     * @return const QString& The unique ID string.
     */
    [[nodiscard]] auto getId() const -> const QString&
    {
        return m_id;
    }

   signals:
    /**
     * @brief Emitted when the tab's metadata (title or icon) changes.
     * @details The Tab Manager should connect to this to refresh the tab bar UI.
     */
    void updated();

   protected:
    /**
     * @brief Unique identifier for the tab instance.
     */
    QString m_id;

    /**
     * @brief Display title used by the UI container.
     */
    QString m_title;

    /**
     * @brief The Icon next to the display Title.
     */
    QIcon m_icon;
};

}  // namespace Core
/**
 * @brief Register the interface pointer with Qt's Meta-Type system.
 * @details This allows Core::ITabComponent* to be stored inside a QVariant,
 * which is required for the ComponentRole in the model's data() method.
 */
Q_DECLARE_METATYPE(Core::ITabComponent*)
