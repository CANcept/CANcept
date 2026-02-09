#pragma once

#include <QHBoxLayout>
#include <QStringList>
#include <QWidget>

namespace Logging {

/**
 * @class StatusTagsContainer
 * @brief Container widget for displaying message status tags during recording.
 *
 * This component manages a horizontal layout of styled labels representing
 * the messages being logged in the current session.
 */
class StatusTagsContainer final : public QWidget
{
    Q_OBJECT

   public:
    explicit StatusTagsContainer(QWidget* parent = nullptr);
    ~StatusTagsContainer() override = default;

    /**
     * @brief Updates the status tags shown during recording.
     * @param messages List of message IDs/names to display as tags
     */
    void updateStatusTags(const QStringList& messages);

   private:
    QHBoxLayout* m_layout;
};

}  // namespace Logging
