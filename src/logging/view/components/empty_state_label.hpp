#pragma once

#include <QLabel>

namespace Logging {

/**
 * @class EmptyStateLabel
 * @brief Styled label shown when no logging sessions exist.
 *
 * This component provides a consistent styled placeholder label
 * displayed in the center when the history table is empty.
 */
class EmptyStateLabel final : public QLabel
{
    Q_OBJECT

   public:
    explicit EmptyStateLabel(QWidget* parent = nullptr);
    ~EmptyStateLabel() override = default;

   private:
    void applyStyle();
};

}  // namespace Logging
