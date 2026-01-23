#include "youtube_component.hpp"

#include <QUrl>
#include <QWebEngineFullScreenRequest>
#include <QWebEnginePage>

namespace YouTube {

YoutubeComponent::YoutubeComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, "youtube_tab", "YouTube"), m_videoId("dQw4w9WgXcQ")
{
    setupUi();
}

YoutubeComponent::~YoutubeComponent() = default;

auto YoutubeComponent::getView() -> QWidget*
{
    return m_view.get();
}

void YoutubeComponent::setupUi()
{
    m_view = std::make_unique<QWidget>();

    auto* layout = new QVBoxLayout(m_view.get());
    layout->setContentsMargins(0, 0, 0, 0);

    // YouTube Web View
    m_webView = new QWebEngineView(m_view.get());
    m_webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_webView->setMinimumSize(640, 480);

    // Configure web engine settings for video playback
    auto* settings = m_webView->page()->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);

    // Connect fullscreen request handler
    connect(m_webView->page(), &QWebEnginePage::fullScreenRequested, this,
            &YoutubeComponent::handleFullScreenRequest);

    layout->addWidget(m_webView);

    // Load video immediately
    loadVideo();
}

void YoutubeComponent::onStart()
{
    loadVideo();
}

void YoutubeComponent::onStop()
{
    // Stop the video to save resources when tab is deactivated
    if (m_webView)
    {
        m_webView->setUrl(QUrl("about:blank"));
    }
}

void YoutubeComponent::loadVideo()
{
    // Load the full YouTube website
    m_webView->setUrl(QUrl("https://www.youtube.com"));
}

void YoutubeComponent::handleFullScreenRequest(QWebEngineFullScreenRequest request)
{
    request.accept();

    if (request.toggleOn())
    {
        m_webView->showFullScreen();
    } else
    {
        m_webView->showNormal();
    }
}

}  // namespace YouTube
