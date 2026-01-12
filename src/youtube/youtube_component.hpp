#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QtWebEngineWidgets/QWebEngineView>
#include <string>
#include <vector>
#include <random>

#include "/core/interface/i_tab_component.hpp"
#include "/core/interface/i_event_broker.hpp"

namespace YouTube {

class YoutubeComponent : public ITabComponent, public QWidget {
    Q_OBJECT

public:
    /**
     * @param broker The EnTT-based EventBroker.
     * @param id Unique ID for this tab instance.
     */
    explicit YoutubeComponent(IEventBroker& broker, QString id, QWidget* parent = nullptr);
    
    ~YoutubeComponent() override = default;

    // --- ITabComponent Implementation ---
    auto getView() -> QWidget* override { return this; }

    // --- ILifecycle Implementation ---
    void onStart() override;
    void onStop() override;

private:
    void setupUi();
    void loadRandomVideo();

    QWebEngineView* m_webView{nullptr};
    
    const std::vector<QString> m_videoPool = {
        "dQw4w9WgXcQ", "jfKfPfyJRdk", "9bZkp7q19f0", "tQ0yjYUFKAE"
    };
};

} // namespace YouTube
