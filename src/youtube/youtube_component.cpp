
#include "youtube_component.hpp"

YoutubeComponent::YoutubeComponent(EventBroker& eventBroker, QWidget* parent)
    : QWidget(parent)
    , m_eventBroker(eventBroker)
{
    setupUi();
    loadRandomVideo();
}

void YoutubeComponent::setupUi() {
    auto* layout = new QVBoxLayout(this);

    // Header Title
    auto* titleLabel = new QLabel("CAN Bus Manager - YouTube Tab", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    // WebView for YouTube
    m_webView = new QWebEngineView(this);
    
    // Add to layout
    layout->addWidget(titleLabel);
    layout->addWidget(m_webView);
    
    setLayout(layout);
}

void YoutubeComponent::loadRandomVideo() {
    // Random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, m_videoPool.size() - 1);
    
    std::string videoId = m_videoPool[dis(gen)];
    
    // Construct the embed URL
    // Use "https://www.youtube.com/embed/" for a clean, full-frame view
    QString url = QString("https://www.youtube.com/embed/%1").arg(QString::fromStdString(videoId));
    
    m_webView->setUrl(QUrl(url));
}
