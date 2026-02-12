#pragma once

#include <QFrame>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

namespace Core {

/**
 * @class CardWidget
 * @brief Reusable card container with optional icon, title, and subtitle.
 *
 * Provides a styled card frame that can be used across modules for consistent UI.
 * Content can be added to the card's layout via contentLayout().
 */
class CardWidget final : public QFrame
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a card widget.
     * @param title The card title (optional).
     * @param subtitle The card subtitle (optional).
     * @param iconPath Path to an SVG icon displayed next to the title (optional).
     * @param parent Parent widget.
     */
    explicit CardWidget(const QString& title = QString(), const QString& subtitle = QString(),
                        const QString& iconPath = QString(), QWidget* parent = nullptr);

    /**
     * @brief Setter for the title.
     * @param title
     */
    void setTitle(const QString& title);

    /**
     * Setter for the subtitle
     * @param subtitle
     */
    void setSubtitle(const QString& subtitle);
    ~CardWidget() override = default;

    /**
     * @brief Returns the content layout for adding widgets to the card.
     * @return Pointer to the card's vertical layout.
     */
    [[nodiscard]] auto contentLayout() const -> QVBoxLayout*
    {
        return m_contentLayout;
    }

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi(const QString& title, const QString& subtitle, const QString& iconPath);
    void applyStyle();

    QVBoxLayout* m_contentLayout;
    QLabel* m_iconLabel{nullptr};
    QLabel* m_titleLabel{nullptr};
    QLabel* m_subtitleLabel{nullptr};
    QString m_iconPath;
};

}  // namespace Core
