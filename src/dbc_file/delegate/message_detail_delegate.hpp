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

/**
 * @file message_detail_delegate.hpp
 * @brief Custom item delegate that renders a signal detail "card" in the messages detail view.
 *
 * The delegate draws a card-like layout consisting of:
 * - Header: signal name and optional unit badge
 * - Grid: two rows of attribute/value pairs (start bit, length, byte order, type, factor, offset,
 * min, max)
 * - Footer: receiver list (if available)
 *
 * Data is retrieved using Qt standard roles (DisplayRole) and custom DBC roles
 * defined in dbc_roles.hpp.
 */

#pragma once

#include <QStyledItemDelegate>

class QPainter;
class QStyleOptionViewItem;
class QModelIndex;
class QSize;
class QRect;
class QString;

namespace DbcFile {

/**
 * @class MessagesDetailDelegate
 * @brief Paints a card-style detail row for a selected message/signal.
 *
 * The delegate is intended for detail panes where a richer, multi-line representation
 * is needed. It uses the current theme for spacing and colors and relies on
 * Core::ItemPainter utilities for consistent UI rendering.
 */
class MessagesDetailDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the delegate.
     * @param parent Optional QObject parent.
     */
    explicit MessagesDetailDelegate(QObject* parent = nullptr);

    /**
     * @brief Returns the size hint for a single card item.
     * @param option Style options.
     * @param index Model index (unused for sizing).
     * @return Size hint with a fixed height and view-defined width.
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
        -> QSize override;

    /**
     * @brief Paints the card layout for the given model index.
     * @param painter Painter used by the view.
     * @param option Style options (rect, state).
     * @param index Model index providing the data to display.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

   private:
    /** @brief Draws the header (name + optional unit badge). */
    static void drawHeader(QPainter* painter, const QRect& rect, const QModelIndex& index);

    /** @brief Draws the attribute grid (two rows, four columns). */
    static void drawGrid(QPainter* painter, const QRect& rect, const QModelIndex& index);

    /**
     * @brief Draws a single label/value pair within a grid cell.
     * @param painter Painter used by the view.
     * @param rect Cell rectangle.
     * @param label Attribute label.
     * @param value Attribute value.
     */
    static void drawAttributePair(QPainter* painter, const QRect& rect, const QString& label,
                                  const QString& value);

    /** @brief Draws the footer (receiver list) if available. */
    static void drawFooter(QPainter* painter, const QRect& rect, const QModelIndex& index);
};

}  // namespace DbcFile
