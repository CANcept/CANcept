#include "message_detail_delegate.hpp"

#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace DbcFile {

// -----------------------------------------------------------------------------
// Layout helpers
// -----------------------------------------------------------------------------
namespace Layout {

/**
 * @brief Card outer margin (space between delegate items).
 */
inline auto outerMarginPx() -> int
{
    return THEME.spacing().spacingSm;
}

/**
 * @brief Inner padding of the card content.
 */
inline auto cardPaddingPx() -> int
{
    return THEME.spacing().spacingMd;
}

/**
 * @brief Gap between header/grid/footer blocks.
 */
inline auto sectionGapPx() -> int
{
    return THEME.spacing().spacingSm;
}

inline auto headerHeightPx() -> int
{
    return THEME.spacing().HeightSm;
}

inline auto gridRowHeightPx() -> int
{
    return THEME.spacing().HeightMd;
}

inline auto footerHeightPx() -> int
{
    return THEME.spacing().HeightSm;
}

/**
 * @brief Total delegate height for a single card.
 *
 * Header + gap + 2 grid rows + gap + footer + paddings.
 */
inline auto totalHeightPx() -> int
{
    return cardPaddingPx()
         + headerHeightPx()
         + sectionGapPx()
         + (2 * gridRowHeightPx())
         + sectionGapPx()
         + footerHeightPx()
         + cardPaddingPx();
}

} // namespace Layout

namespace {

/**
 * @brief Formats numeric values consistently for detail cards.
 */
inline auto formatNumber(double value) -> QString
{
    return QString::number(value, 'g', 12);
}

} // namespace

MessagesDetailDelegate::MessagesDetailDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

auto MessagesDetailDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex&) const -> QSize
{
    return {option.rect.width(), Layout::totalHeightPx()};
}

void MessagesDetailDelegate::paint(QPainter* painter,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    // Compute card rectangle (slightly reduced to visually separate items).
    const QRect cardRect = option.rect.adjusted(
        0, 0, 0, -Layout::outerMarginPx());

    // Paint card background.
    Core::ItemPainter::paintCard(painter, cardRect, false);

    // Content area.
    const int padding = Layout::cardPaddingPx();
    const int contentLeft = cardRect.left() + padding;
    const int contentWidth = cardRect.width() - 2 * padding;

    int cursorY = cardRect.top() + padding;

    // A) Header
    const QRect headerRect(contentLeft, cursorY, contentWidth, Layout::headerHeightPx());
    drawHeader(painter, headerRect, index);
    cursorY += Layout::headerHeightPx() + Layout::sectionGapPx();

    // B) Grid (2 rows)
    const int gridHeight = 2 * Layout::gridRowHeightPx();
    const QRect gridRect(contentLeft, cursorY, contentWidth, gridHeight);
    drawGrid(painter, gridRect, index);
    cursorY += gridHeight + Layout::sectionGapPx();

    // C) Footer
    const QRect footerRect(contentLeft, cursorY, contentWidth, Layout::footerHeightPx());
    drawFooter(painter, footerRect, index);
}

void MessagesDetailDelegate::drawHeader(QPainter* painter,
                                       const QRect& rect,
                                       const QModelIndex& index)
{
    const QString name = index.data(Qt::DisplayRole).toString();
    const QString unit = index.data(DbcRoles::Role_Unit).toString();

    int rightCursor = rect.right();

    // Unit badge (right-aligned).
    if (!unit.isEmpty()) {
        const QSize badgeSize = Core::ItemPainter::measureBadge(unit);
        const QRect badgeRect(
            rightCursor - badgeSize.width(),
            rect.center().y() - badgeSize.height() / 2,
            badgeSize.width(),
            badgeSize.height());

        Core::ItemPainter::paintBadge(painter, badgeRect, unit);
        rightCursor -= (badgeSize.width() + Layout::cardPaddingPx());
    }

    // Name (remaining space).
    const QRect nameRect(rect.left(), rect.top(), rightCursor - rect.left(), rect.height());

    painter->save();
    QFont font = painter->font();
    font.setBold(false);
    font.setPixelSize(THEME.spacing().fontSizeMd);
    painter->setFont(font);
    painter->setPen(THEME.colors().textPrimary);

    const QString elided = painter->fontMetrics().elidedText(
        name, Qt::ElideRight, nameRect.width());

    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elided);
    painter->restore();
}

void MessagesDetailDelegate::drawGrid(QPainter* painter,
                                     const QRect& rect,
                                     const QModelIndex& index) const
{
    const int colWidth = rect.width() / 4;

    // Row 0
    const QString startBit = index.data(Role_StartBit).toString();
    const QString length   = index.data(Role_BitLength).toString()
                           + Constants::SignalsPage::LengthUnit;
    const QString byteOrder = index.data(Role_ByteOrder).toString();
    const QString valueType = index.data(Role_ValueType).toString();

    // Row 1
    const QString factor = formatNumber(index.data(Role_Factor).toDouble());
    const QString offset = formatNumber(index.data(Role_Offset).toDouble());
    const QString minVal = formatNumber(index.data(Role_Min).toDouble());
    const QString maxVal = formatNumber(index.data(Role_Max).toDouble());

    const int rowH = Layout::gridRowHeightPx();
    const int y0 = rect.top();
    const int y1 = rect.top() + rowH;

    // Row 0 cells
    drawAttributePair(painter, QRect(rect.left() + 0 * colWidth, y0, colWidth, rowH),
                      Constants::Headers::SigStartBit, startBit);
    drawAttributePair(painter, QRect(rect.left() + 1 * colWidth, y0, colWidth, rowH),
                      Constants::Headers::SigLength, length);
    drawAttributePair(painter, QRect(rect.left() + 2 * colWidth, y0, colWidth, rowH),
                      Constants::Headers::SigByteOrder, byteOrder);
    drawAttributePair(painter, QRect(rect.left() + 3 * colWidth, y0, colWidth, rowH),
                      Constants::Headers::SigType, valueType);

    // Row 1 cells
    drawAttributePair(painter, QRect(rect.left() + 0 * colWidth, y1, colWidth, rowH),
                      Constants::Headers::SigFactor, factor);
    drawAttributePair(painter, QRect(rect.left() + 1 * colWidth, y1, colWidth, rowH),
                      Constants::Headers::SigOffset, offset);
    drawAttributePair(painter, QRect(rect.left() + 2 * colWidth, y1, colWidth, rowH),
                      Constants::Headers::SigMin, minVal);
    drawAttributePair(painter, QRect(rect.left() + 3 * colWidth, y1, colWidth, rowH),
                      Constants::Headers::SigMax, maxVal);
}

void MessagesDetailDelegate::drawAttributePair(QPainter* painter,
                                              const QRect& rect,
                                              const QString& label,
                                              const QString& value)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    const int labelHeight = spacing.HeightXs;
    const int valueHeight = spacing.HeightXs;
    const int gap = spacing.spacingXs / 2;

    const int totalH = labelHeight + gap + valueHeight;
    const int startY = rect.center().y() - totalH / 2;

    const QRect labelRect(rect.left(), startY, rect.width(), labelHeight);
    const QRect valueRect(rect.left(), startY + labelHeight + gap, rect.width(), valueHeight);

    // Label
    painter->save();
    QFont lf = painter->font();
    lf.setPixelSize(spacing.fontSizeSm);
    painter->setFont(lf);
    painter->setPen(colors.textSecondary);
    painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignTop, label);
    painter->restore();

    // Value
    Core::ItemPainter::paintText(painter, valueRect, value);
}

void MessagesDetailDelegate::drawFooter(QPainter* painter,
                                       const QRect& rect,
                                       const QModelIndex& index) const
{
    const QString receivers = index.data(DbcRoles::Role_Receivers).toString();
    if (receivers.isEmpty()) return;

    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // NOTE: Prefer "Receivers: %1" formatting in constants to avoid trailing spaces.
    const QString label = Constants::Headers::SigReceivers;

    painter->save();
    QFont f = painter->font();
    f.setPixelSize(spacing.fontSizeSm);
    painter->setFont(f);

    painter->setPen(colors.textSecondary);
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, label);

    const int labelWidth = painter->fontMetrics().horizontalAdvance(label);
    const QRect valueRect = rect.adjusted(labelWidth, 0, 0, 0);

    painter->setPen(colors.textPrimary);
    painter->drawText(valueRect, Qt::AlignLeft | Qt::AlignVCenter, receivers);

    painter->restore();
}

} // namespace DbcFile
