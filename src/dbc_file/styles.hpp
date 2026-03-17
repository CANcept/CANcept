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

#include <QString>

namespace DbcFile::Style {

/**
 * @brief Common reusable styles across all pages.
 */
namespace Common {

/**
 * @brief Returns a stylesheet for a vertical scrollbar.
 * @return QString containing QSS for a vertical scrollbar.
 */
QString verticalScrollBar();

/**
 * @brief Styles the label indicating no items have benn found, shown for empty tables and lists
 * @return QString containing QSS.
 */
QString emptyLabel();

}  // namespace Common

/**
 * @brief Styles for the LoadPage.
 */
namespace LoadPage {

/**
 * @brief Styles the upload zone container.
 * @return QString containing QSS.
 */
QString uploadZone();

/**
 * @brief Styles the instruction label inside the upload zone.
 * @return QString containing QSS.
 */
QString uploadInstruction();

/**
 * @brief Styles the status label for success/error.
 * @param isError true for error style, false for success style.
 * @return QString containing QSS.
 */
QString statusLabel(bool isError);

}  // namespace LoadPage

/**
 * @brief Styles for the OverviewPage.
 */
namespace OverviewPage {

/**
 * @brief Styles the scroll area container.
 * @return QString containing QSS.
 */
QString scrollArea();

/**
 * @brief Styles secondary labels (smaller text).
 * @return QString containing QSS.
 */
QString secondaryLabel();

/**
 * @brief Styles the title of statistics.
 * @return QString containing QSS.
 */
QString statTitle();

/**
 * @brief Styles the value of statistics.
 * @return QString containing QSS.
 */
QString statValue();

}  // namespace OverviewPage

/**
 * @brief Styles for the EcusPage.
 */
namespace EcusPage {

/**
 * @brief Styles the QTreeView for ECUs.
 * @return QString containing QSS.
 */
QString treeStyle();

}  // namespace EcusPage

/**
 * @brief Styles for the MessagesPage.
 */
namespace MessagesPage {

/**
 * @brief Styles the signal list inside the detail view.
 * @return QString containing QSS.
 */
QString detailList();
}  // namespace MessagesPage

}  // namespace DbcFile::Style