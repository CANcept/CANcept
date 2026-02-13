#pragma once

#include <QTreeView>

namespace Logging {

/**
 * @class LogHistoryTable
 * @brief Styled tree view for displaying logging session history.
 *
 * This component provides a consistent styled table view for displaying
 * past logging sessions with proper theming and configuration.
 */
class LogHistoryTable final : public QTreeView
{
    Q_OBJECT

   public:
    explicit LogHistoryTable(QWidget* parent = nullptr);
    ~LogHistoryTable() override = default;

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle();
};

}  // namespace Logging
