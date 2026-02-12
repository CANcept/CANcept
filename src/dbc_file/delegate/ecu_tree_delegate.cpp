#include "ecu_tree_delegate.hpp"

#include <QAbstractProxyModel>
#include <algorithm>

#include "core/enum/dbc_itemtype.hpp"
#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace Core {
enum class DbcItemType;
}
namespace DbcFile {

EcuTreeDelegate::EcuTreeDelegate(QTreeView* view, QObject* parent)
    : QStyledItemDelegate(parent), m_treeView(view)
{
}

// =============================================================================
// 1. LAYOUT LOGIK
// =============================================================================

auto EcuTreeDelegate::getCardRect(const QStyleOptionViewItem& option,
                                  int indentLevel) const -> QRect
{
    const auto& spacing = THEME.spacing();

    QRect rect = getViewportRowRect(option);

    // left indent (Tree Level)
    rect.setLeft(rect.left() + spacing.spacingXl * indentLevel);

    // right margin (identical for all cards)
    rect.setRight(rect.right() - spacing.spacingXl);

    return rect;
}
auto EcuTreeDelegate::calculateLayout(const QRect& fullRect, int signalCount) -> MessageLayout
{
    const auto& spacing = THEME.spacing();
    MessageLayout layout;

    // Outer margins
    int outerMargin = spacing.spacingSm / 2;
    layout.cardRect = fullRect.adjusted(0, outerMargin, 0, -outerMargin);

    // Header height
    layout.headerRect = layout.cardRect;
    layout.headerRect.setHeight(spacing.HeightMd);

    // Content Start (below header)
    int contentTop = layout.headerRect.bottom();

    // Grid Calculation
    // Available width mins padding left/right within card
    int availableWidth = layout.cardRect.width() - (2 * spacing.spacingMd);

    // Minimum width per signal item
    int minItemWidth = spacing.WidthSm;
    int gap = spacing.spacingSm;

    // Calculate how items fit in available width
    layout.columns = (availableWidth > 0) ? std::max(1, availableWidth / (minItemWidth + gap)) : 1;

    // Calculate how many rows of items are needed
    int rows = (signalCount == 0) ? 0 : (signalCount + layout.columns - 1) / layout.columns;

    // Calculate exact width per item (with stretch)
    layout.itemWidth = (availableWidth - (layout.columns - 1) * gap) / layout.columns;
    layout.itemHeight = spacing.HeightSm;  // Fixed height for signal items

    // Total content height
    int contentHeight = (rows > 0) ? rows * layout.itemHeight + (rows - 1) * gap : 0;

    layout.contentRect = QRect(layout.cardRect.left() + spacing.spacingMd, contentTop,
                               availableWidth, contentHeight);

    // Total message card height
    int bottomPadding = (rows > 0) ? spacing.spacingMd : 0;
    layout.totalHeight =
        (contentTop - layout.cardRect.top()) + contentHeight + bottomPadding + (2 * outerMargin);

    return layout;
}

// =============================================================================
// 2. STANDARD OVERRIDES
// =============================================================================

auto EcuTreeDelegate::sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const -> QSize
{
    const auto& spacing = THEME.spacing();
    if (index.column() > 0) return {0, 0};

    auto type = static_cast<Core::DbcItemType>(index.data(DbcRoles::Role_ItemType).toInt());

    if (type == Core::DbcItemType::Ecu)
    {
        return {option.rect.width(), spacing.HeightMd};
    }

    if (type == Core::DbcItemType::Message)
    {
        int sigCount = index.data(Role_ChildCount).toInt();
        int width = (m_treeView && m_treeView->viewport()) ? m_treeView->viewport()->width()
                                                           : option.rect.width();

        // Dummy Rect for calculation
        auto layout = calculateLayout(QRect(0, 0, width, 0), sigCount);
        return {width, layout.totalHeight};
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

void EcuTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    if (index.column() > 0) return;

    painter->save();

    auto type = static_cast<Core::DbcItemType>(index.data(Role_ItemType).toInt());

    if (type == Core::DbcItemType::Ecu)
    {
        drawEcu(painter, option, index);
    } else if (type == Core::DbcItemType::Message)
    {
        drawMessage(painter, option, index);
    } else
    {
        // Fallback
        QStyledItemDelegate::paint(painter, option, index);
    }

    painter->restore();
}

// =============================================================================
// 3. DRAWING LOGIK
// =============================================================================

void EcuTreeDelegate::drawEcu(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();
    bool selected = option.state & QStyle::State_Selected;

    // 1. Calculate rect with indentation for arrow
    QRect cardRect = getCardRect(option, 1);
    cardRect.adjust(0, spacing.spacingXs / 2, 0, -spacing.spacingXs / 2);

    // 2. Background/Card
    Core::ItemPainter::paintCard(painter, cardRect, selected);

    // 3. Icon + Title
    int commonPadding = spacing.spacingMd;
    int iconSize = spacing.IconSm;

    // A) Calculate Icon position
    QRect iconRect(cardRect.left() + commonPadding, cardRect.center().y() - iconSize / 2 + 1,
                   iconSize, iconSize);
    Core::ItemPainter::paintIcon(painter, iconRect, QIcon(Constants::Sidebar::IconEcus), selected);

    // B) Calculate title position
    int titleStartX = iconRect.right() + commonPadding;

    // Define title rect (leave space for badges)
    QRect titleRect = cardRect;
    titleRect.setLeft(titleStartX);
    titleRect.setRight(cardRect.right() - spacing.WidthSm);

    QString title = index.data(Qt::DisplayRole).toString();
    Core::ItemPainter::paintText(painter, titleRect, title, false);

    // 4. Badges (right)
    int cursorRight = cardRect.right() - spacing.spacingMd;  // starting point inner right
    int centerY = cardRect.center().y();

    // Preload icons
    QIcon sigIcon(Constants::Sidebar::IconSignals);
    QIcon msgIcon(Constants::Sidebar::IconMessages);

    // A) Signal Count
    QString sigCount = index.data(Role_EcuTotalSignals).toString();
    QSize sigBadgeSize = Core::ItemPainter::measureBadge(sigCount, sigIcon);
    QRect sigBadgeRect(cursorRight - sigBadgeSize.width(), centerY - sigBadgeSize.height() / 2 + 1,
                       sigBadgeSize.width(), sigBadgeSize.height());

    Core::ItemPainter::paintBadge(painter, sigBadgeRect, sigCount, sigIcon);

    cursorRight -= (sigBadgeSize.width() + spacing.spacingSm);  // move cursor to the left

    // B) Message Count
    QString msgCount = index.data(DbcRoles::Role_ChildCount).toString();
    QSize msgBadgeSize = Core::ItemPainter::measureBadge(msgCount, msgIcon);
    QRect msgBadgeRect(cursorRight - msgBadgeSize.width(), centerY - msgBadgeSize.height() / 2 + 1,
                       msgBadgeSize.width(), msgBadgeSize.height());

    Core::ItemPainter::paintBadge(painter, msgBadgeRect, msgCount, msgIcon);
}

void EcuTreeDelegate::drawMessage(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();
    bool selected = option.state & QStyle::State_Selected;

    // Count child elements for grid calculation
    // trace index back to source model index to get access to signals
    const QAbstractItemModel* model = index.model();
    QModelIndex sourceParent = index;
    while (auto* proxy = qobject_cast<const QAbstractProxyModel*>(model))
    {
        sourceParent = proxy->mapToSource(sourceParent);
        model = proxy->sourceModel();
    }
    int signalCount = model->rowCount(sourceParent);

    // Calculate Layout
    QRect rowRect = getCardRect(option, 2);
    MessageLayout layout = calculateLayout(rowRect, signalCount);

    // Paint card background
    Core::ItemPainter::paintCard(painter, layout.cardRect, selected);

    // Paint header
    drawMessageHeader(painter, layout.headerRect, index);

    // Paint signals (grid loop)
    for (int i = 0; i < signalCount; ++i)
    {
        // Position in grid
        int col = i % layout.columns;
        int row = i / layout.columns;

        int x = layout.contentRect.left() + col * (layout.itemWidth + spacing.spacingXs);
        int y = layout.contentRect.top() + row * (layout.itemHeight + spacing.spacingXs);

        QRect signalRect(x, y, layout.itemWidth, layout.itemHeight);

        // Get index of signal
        QModelIndex sigIdx = model->index(i, 0, sourceParent);

        drawSignalItem(painter, signalRect, sigIdx);
    }
}

void EcuTreeDelegate::drawMessageHeader(QPainter* painter, const QRect& headerRect,
                                        const QModelIndex& index)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    int commonPadding = spacing.spacingMd;
    int iconSize = spacing.IconSm;

    // Header content area
    QRect content = headerRect;
    int centerY = content.center().y();
    // Cursor X: start on the left and shift to right for each element
    int cursorX = content.left() + commonPadding;

    // -------------------------------------------------------------------------
    // 1. ICON (Links, vor dem Namen) -> NEU
    // -------------------------------------------------------------------------
    auto icon = index.data(Qt::DecorationRole).value<QIcon>();

    if (!icon.isNull())
    {
        QRect iconRect(cursorX, centerY - iconSize / 2 + 2, iconSize, iconSize);

        Core::ItemPainter::paintIcon(painter, iconRect, icon, false);

        // Move cursor
        cursorX += iconSize + commonPadding;
    }

    // -------------------------------------------------------------------------
    // 2. MESSAGE NAME
    // -------------------------------------------------------------------------
    QString name = index.data(Qt::DisplayRole).toString();

    // Measure text width manually to move cursor correctly
    painter->save();
    QFont font = painter->font();
    font.setBold(false);
    font.setPixelSize(spacing.fontSizeMd);
    painter->setFont(font);
    QFontMetrics fm(font);
    int nameWidth = fm.horizontalAdvance(name);
    painter->restore();

    QRect nameRect(cursorX, content.top(), nameWidth, content.height());
    Core::ItemPainter::paintText(painter, nameRect, name, false);

    // Shift cursor
    cursorX += nameWidth + spacing.spacingMd;

    // -------------------------------------------------------------------------
    // 3. ID BADGE
    // -------------------------------------------------------------------------
    uint msgId = index.data(DbcRoles::Role_Id).toUInt();
    QString idString = QString("0x%1").arg(msgId, 0, 16).toUpper();

    // Custom Style for ID badge
    Core::ItemPainter::BadgeStyle idStyle;
    idStyle.background = colors.surfaceMain;
    idStyle.text = colors.textPrimary;
    idStyle.border = colors.borderSubtle;

    QSize idSize = Core::ItemPainter::measureBadge(idString);
    QRect idRect(cursorX, centerY - idSize.height() / 2, idSize.width(), idSize.height());

    Core::ItemPainter::paintBadge(painter, idRect, idString, QIcon(), &idStyle);

    // move cursor
    cursorX += idSize.width() + spacing.spacingSm;

    // -------------------------------------------------------------------------
    // 4. SIGNAL COUNT BADGE
    // -------------------------------------------------------------------------
    int childCount = index.data(DbcRoles::Role_ChildCount).toInt();
    QString countStr = QString::number(childCount);
    QIcon sigIcon(Constants::Sidebar::IconSignals);

    QSize countSize = Core::ItemPainter::measureBadge(countStr, sigIcon);
    QRect countRect(cursorX, centerY - countSize.height() / 2, countSize.width(),
                    countSize.height());

    // Standard Style
    Core::ItemPainter::paintBadge(painter, countRect, countStr, sigIcon);
}

void EcuTreeDelegate::drawSignalItem(QPainter* painter, const QRect& rect,
                                     const QModelIndex& sigIdx)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // Background: mini card
    Core::ItemPainter::paintCard(painter, rect, false);

    // Padding within mini card
    int padding = spacing.spacingSm;
    int centerY = rect.center().y();

    // Get data
    const QString name = sigIdx.data(Qt::DisplayRole).toString();
    const QString unit = sigIdx.data(Role_Unit).toString();
    const double min = sigIdx.data(Role_Min).toDouble();
    const double max = sigIdx.data(Role_Max).toDouble();

    // Format Range text: [min, max]
    // Strip unnecessary zeros
    QString rangeText =
        QString("[%1, %2]").arg(QString::number(min, 'g', 12), QString::number(max, 'g', 12));

    // -------------------------------------------------------------------------
    // BUILD LAYOUT RIGHT TO LEFT
    // -------------------------------------------------------------------------

    // Start-Cursor right -
    int cursorX = rect.right() - padding;

    // 1. UNIT BADGE
    if (!unit.isEmpty())
    {
        QSize badgeSize = Core::ItemPainter::measureBadge(unit);

        // vertically centered
        QRect badgeRect(cursorX - badgeSize.width(), centerY - badgeSize.height() / 2 + 1,
                        badgeSize.width(), badgeSize.height());

        Core::ItemPainter::paintBadge(painter, badgeRect, unit);

        // Leftshift cursor
        cursorX -= (badgeSize.width() + spacing.spacingSm);
    }

    // 2. RANGE TEXT
    // Measure text width
    painter->save();
    QFont font = painter->font();
    font.setPixelSize(spacing.fontSizeSm);
    painter->setFont(font);
    QFontMetrics fm(font);
    int puffer = spacing.spacingXs;
    int rangeWidth = fm.horizontalAdvance(rangeText) + puffer;
    painter->restore();

    QRect rangeRect(cursorX - rangeWidth, rect.top(), rangeWidth, rect.height());

    // Paint range text
    Core::ItemPainter::paintText(painter, rangeRect, rangeText, false, colors.textPrimary,
                                 Qt::AlignRight | Qt::AlignVCenter);

    // Leftshift cursor
    cursorX -= (rangeWidth + spacing.spacingMd);

    // 3. SIGNAL NAME
    // starts left and can only go to cursorX
    int nameStartX = rect.left() + padding;
    int maxNameWidth = cursorX - nameStartX;

    if (maxNameWidth > 0)
    {
        QRect nameRect(nameStartX, rect.top(), maxNameWidth, rect.height());
        QString displayName = name + ":";
        Core::ItemPainter::paintText(painter, nameRect, displayName, false, colors.textPrimary,
                                     Qt::AlignLeft | Qt::AlignVCenter);
    }
}

auto EcuTreeDelegate::getViewportRowRect(const QStyleOptionViewItem& option) const -> QRect
{
    QRect rect = option.rect;
    if (m_treeView && m_treeView->viewport())
    {
        rect.setLeft(0);
        rect.setWidth(m_treeView->viewport()->width());
    }
    return rect;
}

}  // namespace DbcFile