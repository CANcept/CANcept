/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
