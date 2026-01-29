//
// Created by Adrian Rupp on 28.01.26.
//
#pragma once
#include <QLabel>
#include <QWidget>
namespace Core {
/**
 * @class SectionHeader
 * @brief A standard header widget displaying a Title and an optional Subtitle.
 *
 * @details
 * Used at the top of content cards or pages to provide context.
 * - Title: Large, bold text (Primary Color).
 * - Subtitle: Smaller, normal text (Secondary Color).
 * Uses the ThemeManager to ensure consistent styling across the app.
 */
class SectionHeader : public QWidget {
    Q_OBJECT

public:
    /**
     * @param title The main heading text.
     * @param subtitle Optional descriptive text below the title.
     * @param parent Parent widget.
     */
    explicit SectionHeader(const QString& title, const QString& subtitle = "", QWidget* parent = nullptr);

    /**
     * @brief Updates the title text.
     */
    void setTitle(const QString& title);

    /**
     * @brief Updates the subtitle text. Hides the label if empty.
     */
    void setSubtitle(const QString& subtitle);

private:
    void setupUi();

    QString m_titleText;
    QString m_subtitleText;

    QLabel* m_lblTitle;
    QLabel* m_lblSubtitle;
};

}
