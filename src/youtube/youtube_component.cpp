#include "youtube_component.hpp"

#include <QUrl>

namespace YouTube {

YoutubeComponent::YoutubeComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, "youtube_tab", "YouTube"),
      m_videoId("dQw4w9WgXcQ")
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

    layout->addWidget(m_webView);
}

void YoutubeComponent::onStart()
{
    loadVideo();
}

void YoutubeComponent::onStop()
{
    // Stop the video to save resources when tab is deactivated
    if (m_webView) {
        m_webView->setUrl(QUrl("about:blank"));
    }
}

void YoutubeComponent::loadVideo()
{
    if (m_videoId.isEmpty()) {
        return;
    }

    // Use the embed URL for cleaner playback without YouTube UI clutter
    QString url = QString("https://www.youtube.com/embed/%1?autoplay=0").arg(m_videoId);
    m_webView->setUrl(QUrl(url));
}

}  // namespace YouTube
