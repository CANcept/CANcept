#pragma once

#include <QTreeView>

namespace Logging {

/**
 * @class HistoryTable
 * @brief Styled tree view for displaying logging session history.
 *
 * This component provides a consistent styled table view for displaying
 * past logging sessions with proper theming and configuration.
 */
class HistoryTable final : public QTreeView
{
    Q_OBJECT

   public:
    explicit HistoryTable(QWidget* parent = nullptr);
    ~HistoryTable() override = default;

   private:
    void setupUi();
    void applyStyle();
};

}  // namespace Logging
