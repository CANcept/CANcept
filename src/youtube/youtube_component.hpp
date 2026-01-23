#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QtWebEngineWidgets/QWebEngineView>
#include <memory>

#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"

namespace YouTube {

/**
 * @class YoutubeComponent
 * @brief A tab component that displays a single YouTube video.
 *
 * This component embeds a YouTube video using QWebEngineView.
 * It implements the ITabComponent interface to integrate with the AppRoot.
 */
class YoutubeComponent final : public Core::ITabComponent
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the YouTube component.
     * @param broker Reference to the event broker for communication.
     * @param videoId The YouTube video ID to display.
     */
    explicit YoutubeComponent(Core::IEventBroker& broker);

    /**
     * @brief Destructor.
     */
    ~YoutubeComponent() override;

    /**
     * @brief Returns the main widget for display in the application window.
     * @return QWidget* Pointer to the view widget.
     */
    auto getView() -> QWidget* override;

    /**
     * @brief Called when the application starts/module is activated.
     * Loads the YouTube video.
     */
    void onStart() override;

    /**
     * @brief Called when the application stops/module is deactivated.
     * Stops video playback to save resources.
     */
    void onStop() override;

   private:
    /**
     * @brief Sets up the UI layout and widgets.
     */
    void setupUi();

    /**
     * @brief Loads the configured YouTube video.
     */
    void loadVideo();

    /** @brief The YouTube video ID to display. */
    QString m_videoId;

    /** @brief The root widget containing the UI. */
    std::unique_ptr<QWidget> m_view;

    /** @brief The web view for displaying the YouTube embed. */
    QWebEngineView* m_webView{nullptr};
};

}  // namespace YouTube
