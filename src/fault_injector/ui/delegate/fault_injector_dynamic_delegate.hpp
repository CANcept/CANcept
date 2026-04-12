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
#include <QStringList>
#include <QStyledItemDelegate>
#include <functional>

namespace FaultInjector {

/**
 * @class FaultInjectorDynamicDelegate
 * @brief This class handles the rendering of the column effect and triggers inside a fault row
 */
class FaultInjectorDynamicDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
   public:
    explicit FaultInjectorDynamicDelegate(std::function<QStringList(const QVariant&)> extractor,
                                          QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   private:
    std::function<QStringList(const QVariant&)> m_extractor;

    void paintChip(QPainter* painter, const QRect& rect, const QString& label) const;
    void paintOverflow(QPainter* painter, const QRect& rect, int count) const;
};

}  // namespace FaultInjector