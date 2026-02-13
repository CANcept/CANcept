#pragma once

#include <QLabel>

namespace Logging {

/**
 * @class NoLogsLabel
 * @brief Styled label shown when no logging sessions exist.
 *
 * This component provides a consistent styled placeholder label
 * displayed in the center when the history table is empty.
 */
class NoLogsLabel final : public QLabel
{
    Q_OBJECT

   public:
    explicit NoLogsLabel(QWidget* parent = nullptr);
    ~NoLogsLabel() override = default;

   private:
    void applyStyle();
};

}  // namespace Logging
