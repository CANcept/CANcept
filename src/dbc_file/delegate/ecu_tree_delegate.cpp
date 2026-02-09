#include "ecu_tree_delegate.hpp"

#include <QTreeView>

namespace DbcFile {
EcuTreeDelegate::EcuTreeDelegate(QTreeView* view, QObject* parent)
    : QStyledItemDelegate(parent), m_treeView(view) {}

// =============================================================================
// 1. LAYOUT ENGINE
// =============================================================================

auto EcuTreeDelegate::calculateLayout(const QRect& fullRect, int signalCount) -> MessageLayout
{
    const auto& spacing = THEME.spacing();
    MessageLayout layout;

    // Outer margin
    int outerMargin = spacing.spacingSm / 2;
    layout.cardRect = fullRect.adjusted(outerMargin, outerMargin, -outerMargin, -outerMargin);

    // Header
    layout.headerRect = layout.cardRect;
    layout.headerRect.setHeight(spacing.HeightMd);

    // Content start
    const int contentTop = layout.headerRect.bottom() + spacing.spacingXs;

    // Grid calculation
    int availableWidth = layout.cardRect.width() - 2 * spacing.spacingMd;
    int minItemWidth = spacing.WidthSm;
    int gapH = spacing.spacingXs;
    int gapV = spacing.spacingXs;

    layout.columns = (availableWidth > 0) ? std::max(1, availableWidth / (minItemWidth + gapH)) : 1;
    int rows = (signalCount == 0) ? 0 : (signalCount + layout.columns - 1) / layout.columns;

    layout.itemWidth = (availableWidth - (layout.columns - 1) * gapH) / layout.columns;
    layout.itemHeight = spacing.HeightMd;

    int contentHeight = (rows > 0) ? rows * layout.itemHeight + (rows - 1) * gapV : 0;
    layout.contentRect = QRect(layout.cardRect.left() + spacing.spacingMd,
                               contentTop,
                               availableWidth,
                               contentHeight);

    layout.totalHeight = (contentTop - layout.cardRect.top()) + contentHeight + spacing.spacingMd + 2 * outerMargin;

    return layout;
}

// =============================================================================
// 2. OVERRIDES
// =============================================================================

void EcuTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    if (index.column() > 0) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const auto type = static_cast<Core::DbcItemType>(index.data(DbcRoles::Role_ItemType).toInt());

    if (type == Core::DbcItemType::Ecu)
        drawEcu(painter, option, index);
    else if (type == Core::DbcItemType::Message)
        drawMessage(painter, option, index);

    painter->restore();
}

auto EcuTreeDelegate::sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize
{
    if (index.column() > 0) return {0, 0};

    const auto type = static_cast<Core::DbcItemType>(index.data(Role_ItemType).toInt());

    if (type == Core::DbcItemType::Ecu)
        return {option.rect.width(), THEME.spacing().HeightLg / 2};

    if (type == Core::DbcItemType::Message) {
        int sigCount = index.data(Role_ChildCount).toInt();
        int width = m_treeView && m_treeView->viewport() ? m_treeView->viewport()->width() : option.rect.width();
        auto layout = calculateLayout(QRect(0, 0, width, 0), sigCount);
        return {width, layout.totalHeight};
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

// =============================================================================
// 3. DRAW HELPERS
// =============================================================================

void EcuTreeDelegate::drawMessage(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();

    QRect fullRect = getViewportRowRect(option);
    fullRect.setLeft(fullRect.left() + spacing.spacingXl);

    // Resolve source model behind proxy
    const QAbstractItemModel* model = index.model();
    QModelIndex sourceParent = index;
    while (auto* proxy = qobject_cast<const QAbstractProxyModel*>(model)) {
        sourceParent = proxy->mapToSource(sourceParent);
        model = proxy->sourceModel();
    }
    int signalCount = model->rowCount(sourceParent);

    // Calculate layout
    MessageLayout layout = calculateLayout(fullRect, signalCount);

    // Draw card background
    Core::ItemPainter::paintCardBackground(painter, layout.cardRect, false);

    // Draw header
    drawMessageHeader(painter, layout.headerRect, index);

    // Draw signals
    for (int i = 0; i < signalCount; ++i) {
        QModelIndex sigIdx = model->index(i, 0, sourceParent);
        int col = i % layout.columns;
        int row = i / layout.columns;

        int x = layout.contentRect.left() + col * (layout.itemWidth + spacing.spacingXs);
        int y = layout.contentRect.top() + row * (layout.itemHeight + spacing.spacingXs);

        QRect sigRect(x, y, layout.itemWidth, layout.itemHeight);
        drawSignalItem(painter, sigRect, sigIdx);
    }
}

void EcuTreeDelegate::drawMessageHeader(QPainter* painter, const QRect& headerRect,
                                        const QModelIndex& index)
{
    const auto& spacing = THEME.spacing();
    const auto& colors  = THEME.colors();

    QString name = index.data(Qt::DisplayRole).toString();
    uint msgId = index.data(Role_Id).toUInt();
    QString idStr = QString("0x%1").arg(msgId, 0, 16).toUpper();

    // Font setup
    QFont font = painter->font();
    font.setPointSize(spacing.fontSizeXs);
    painter->setFont(font);
    painter->setPen(colors.textPrimary);

    // Draw name vertically centered
    QRect textRect = headerRect;
    textRect.setLeft(textRect.left() + spacing.spacingXl);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, name);

    // Draw badge next to name
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(name);
    QRect badgeAnchor = textRect;
    badgeAnchor.setLeft(textRect.left() + textWidth + spacing.spacingMd);
    drawBadge(painter, badgeAnchor, idStr, colors.surfaceMain, colors.textSecondary, true);
}

void EcuTreeDelegate::drawSignalItem(QPainter* painter, const QRect& rect,
                                     const QModelIndex& sigIdx)
{
    Core::ItemPainter::paintCardBackground(painter, rect, false);

    auto icon = sigIdx.data(Qt::DecorationRole).value<QIcon>();
    QString title = sigIdx.data(Qt::DisplayRole).toString() + ":";
    QString unit = sigIdx.data(Role_Unit).toString();

    double min = sigIdx.data(Role_Min).toDouble();
    double max = sigIdx.data(Role_Max).toDouble();
    auto formatNum = [](double v) {
        QString s = QString::number(v, 'f', 2);
        while (s.endsWith('0')) s.chop(1);
        if (s.endsWith('.')) s.chop(1);
        return s;
    };
    QString rangeText = QString("[%1, %2]").arg(formatNum(min), formatNum(max));

    // Draw content
    Core::ItemPainter::paintIcon(painter, rect, icon, false);
    Core::ItemPainter::paintTitle(painter, rect, title);

    int badgeWidth = 0;
    if (!unit.isEmpty())
        badgeWidth = Core::ItemPainter::paintBadge(painter, rect, unit, QIcon());

    if (!rangeText.isEmpty())
        Core::ItemPainter::paintDetailText(painter, rect, rangeText, badgeWidth);
}

void EcuTreeDelegate::drawEcu(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();
    const auto& colors  = THEME.colors();

    QRect ecuRect = getViewportRowRect(option).adjusted(spacing.spacingXl,
                                                        spacing.spacingXs, 0, -spacing.spacingXs);

    painter->setBrush(colors.surfaceMain);
    painter->setPen(QPen(colors.borderSubtle, 1));
    painter->drawRoundedRect(ecuRect, spacing.radiusSm, spacing.radiusSm);

    Core::ItemPainter::paintIcon(painter, ecuRect, QIcon(Constants::Sidebar::IconEcus), false);
    Core::ItemPainter::paintTitle(painter, ecuRect, index.data(Qt::DisplayRole).toString(), false);

    QRect badgeArea = ecuRect;
    int widthFirstBadge = Core::ItemPainter::paintBadge(painter, badgeArea,
                                                        index.data(Role_EcuTotalSignals).toString(),
                                                        QIcon(Constants::Sidebar::IconSignals));
    badgeArea.setRight(badgeArea.right() - widthFirstBadge - spacing.spacingSm);

    Core::ItemPainter::paintBadge(painter, badgeArea,
                                  index.data(Role_ChildCount).toString(),
                                  QIcon(Constants::Sidebar::IconMessages));
}

void EcuTreeDelegate::drawBadge(QPainter* painter, const QRect& anchorRect,
                                const QString& text, const QColor& bg, const QColor& fg,
                                bool border)
{
    const auto& spacing = THEME.spacing();
    const auto& colors  = THEME.colors();

    QFont font = painter->font();
    font.setPointSize(spacing.fontSizeXs);
    painter->setFont(font);

    QFontMetrics fm(font);
    int badgeWidth = fm.horizontalAdvance(text) + 2 * spacing.spacingXs;
    int badgeHeight = spacing.HeightXs;

    // Vertical center
    int yPos = anchorRect.center().y() - (badgeHeight / 2);
    if (anchorRect.height() % 2 != badgeHeight % 2) yPos++;

    QRect badgeRect(anchorRect.left(), yPos, badgeWidth, badgeHeight);

    painter->save();
    painter->setBrush(bg);
    painter->setPen(border ? QPen(colors.borderSubtle) : Qt::NoPen);
    painter->drawRoundedRect(badgeRect, spacing.radiusXs / 2.0, spacing.radiusXs / 2.0);
    painter->setPen(fg);
    painter->drawText(badgeRect, Qt::AlignCenter, text);
    painter->restore();
}

auto EcuTreeDelegate::getViewportRowRect(const QStyleOptionViewItem& option) const -> QRect
{
    QRect rect = option.rect;
    if (m_treeView && m_treeView->viewport()) {
        rect.setLeft(0);
        rect.setWidth(m_treeView->viewport()->width());
    }
    return rect;
}

} // namespace DbcFile