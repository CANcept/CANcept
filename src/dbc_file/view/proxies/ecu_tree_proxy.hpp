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

#include <QSortFilterProxyModel>
#include <QString>

namespace Core {
enum class DbcItemType;
}

namespace DbcFile {

/**
 * @brief Proxy model for filtering ECUs and Messages in a tree view.
 *
 * Provides:
 * - Text search filtering
 * - Category-based filtering (active/passive/all) using integers
 * - Custom child handling (Messages are treated as leaf nodes)
 */
class EcuTreeProxy : public QSortFilterProxyModel
{
    Q_OBJECT
   public:
    /**
     * @brief Construct a new EcuTreeProxy.
     * @param parent Optional parent QObject
     */
    explicit EcuTreeProxy(QObject* parent = nullptr);

    [[nodiscard]] auto getSearchFilter() const -> QString
    {
        return m_filterText;
    }
    [[nodiscard]] auto getFilterCategory() const -> int
    {
        return m_filterCategory;
    }
    /**
     * @brief Set the text used for filtering by name.
     * @param text Filter string
     */
    void setSearchText(const QString& text);

    /**
     * @brief Set the category filter using an integer.
     * @param index 0 = all, 1 = active, 2 = passive
     */
    void setFilterCategory(int index);

    /**
     * @brief Overrides default child handling for Messages.
     * Messages are considered leaf nodes.
     * @param parent Parent index
     * @return true if parent has children
     */
    [[nodiscard]] auto hasChildren(const QModelIndex& parent) const -> bool override;

    /**
     * @brief Returns always 1 so there is only on column.
     */
    int columnCount(const QModelIndex& parent) const override;

   protected:
    /**
     * @brief Determines whether a row should be visible based on search text and category.
     * @param sourceRow Row in the source model
     * @param sourceParent Parent index in the source model
     * @return true if row matches current filter
     */
    [[nodiscard]] auto filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
        -> bool override;

   private:
    QString m_filterText;    ///< Current search text
    int m_filterCategory{};  ///< Current category filter (0=all, 1=active, 2=passive)
};

}  // namespace DbcFile