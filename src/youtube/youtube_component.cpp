#include "youtube_component.hpp"

namespace YouTube {

YoutubeComponent::YoutubeComponent(IEventBroker& broker, QString id, QWidget* parent)
    : ITabComponent(broker, std::move(id), "CAN Bus Manager - YouTube Tab"),
      QWidget(parent)
{
    setupUi();
}

void YoutubeComponent::setupUi() {
    auto* layout = new QVBoxLayout(this);

    // Title Label
    auto* titleLabel = new QLabel(m_title, this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // YouTube Web View
    m_webView = new QWebEngineView(this);
    
    layout->addWidget(titleLabel);
    layout->addWidget(m_webView);
    
    // Styling the container
    this->setLayout(layout);
}

void YoutubeComponent::onStart() {
    // We load the video when the lifecycle starts
    loadRandomVideo();
}

void YoutubeComponent::onStop() {
    // Stop the video/browser to save resources when tab is "stopped"
    if (m_webView) {
        m_webView->setUrl(QUrl("about:blank"));
    }
}

void YoutubeComponent::loadRandomVideo() {
    if (m_videoPool.empty()) return;

    // Random selection
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(m_videoPool.size() - 1));
    
    QString videoId = m_videoPool[dis(gen)];
    
    // "embed" allows auto-scaling and bypasses the standard YT UI
    QString url = QString("https://www.youtube.com/embed/%1?autoplay=1").arg(videoId);
    
    m_webView->setUrl(QUrl(url));
}

} // namespace YouTube
