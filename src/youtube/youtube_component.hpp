#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QtWebEngineWidgets/QWebEngineView>
#include <string>
#include <vector>
#include <random>

// Assuming these interfaces exist in your project
#include "/core/interface/i_tab_component.hpp"
#include "/event_broker/event_broker.hpp"

class YoutubeComponent final : public QWidget, public ITabComponent {
    Q_OBJECT

public:
    explicit YoutubeComponent(EventBroker& eventBroker, QWidget* parent = nullptr);
    ~YoutubeComponent() override = default;

private:
    void setupUi();
    void loadRandomVideo();

    EventBroker& m_eventBroker;
    QWebEngineView* m_webView;
    
    // A small pool of sample videos (IDs)
    const std::vector<std::string> m_videoPool = {
        "dQw4w9WgXcQ", "jfKfPfyJRdk", "9bZkp7q19f0", "L_jWHffIx5E"
    };
};
