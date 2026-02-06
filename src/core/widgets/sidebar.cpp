//
// Created by Adrian Rupp on 28.01.26.
//
#include "core/widgets/sidebar.hpp"

#include <qstandarditemmodel.h>

#include <QListView>

#include "core/delegates/sidebar_delegate.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/theme/theme_manager.hpp"
namespace Core {
Sidebar::Sidebar(QWidget* parent) : QListView(parent)
{
    setupUi();
    setupConnections();
}

void Sidebar::setupUi()
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setMaximumWidth(THEME.spacing().WidthSm);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFrameShape(QFrame::NoFrame);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionRectVisible(false);

    m_model = new QStandardItemModel(this);
    setModel(m_model);

    m_delegate = new SidebarDelegate(this);
    setItemDelegate(m_delegate);

    applyStyle();
}

void Sidebar::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    setStyleSheet(QString(R"(
        QListView {
            background-color: %1;
            border-right: %2px solid %3;
            color: %4;
            font-size: %5px;
            outline: 0;
        }

        QListView::item {
            border-radius: %6px;
            padding: %7px;
            margin-right: %8px;
            margin-left: %8px;
        }

        QListView::item:selected {
            background-color: %9;
            color: %10;
        }
    )")
                      .arg(colors.surfaceMain.name(QColor::HexArgb))
                      .arg(spacing.borderThin)
                      .arg(colors.borderSubtle.name(QColor::HexArgb))
                      .arg(colors.textSecondary.name(QColor::HexArgb))
                      .arg(spacing.fontSizeMd)
                      .arg(spacing.radiusSm)
                      .arg(spacing.spacingXl)
                      .arg(spacing.spacingMd)
                      .arg(colors.surfacePrimary.name(QColor::HexArgb))
                      .arg(colors.textPrimary.name(QColor::HexArgb)));
}

bool Sidebar::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QListView::event(event);
}

void Sidebar::setupConnections()
{
    // 1. Click a handling: only send signal if clicked index is valid, respective item is enabled
    connect(this, &QListView::clicked, [this](const QModelIndex& index) {
        if (!index.isValid()) return;
        if (!(index.flags() & Qt::ItemIsEnabled)) return;

        // Send signal
        emit tabSelected(index.row());
    });

    // Avoid empty selection (deselecting when clicking empty space)
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& sel, const QItemSelection& desel) {
                if (sel.isEmpty() && !desel.isEmpty())
                {
                    // Reselect previous selection
                    selectionModel()->select(
                        desel, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                    setCurrentIndex(desel.indexes().first());
                }
            });
}

void Sidebar::setToolTipText(const QString& toolTipText) const
{
    m_delegate->setToolTipText(toolTipText);
}
void Sidebar::addTab(const QIcon& icon, const QString& title, bool enabled)
{
    auto* item = new QStandardItem(icon, title);
    item->setEnabled(enabled);
    item->setSelectable(enabled);
    m_model->appendRow(item);

    // Select first item initially
    if (m_model->rowCount() == 1)
    {
        setCurrentIndex(m_model->index(0, 0));
    }
}

void Sidebar::setNavigationEnabled(const bool enabled) const
{
    for (int i = 1; i < m_model->rowCount(); ++i)
    {
        if (auto* item = m_model->item(i))
        {
            item->setEnabled(enabled);
            item->setSelectable(enabled);
        }
    }
}
}  // namespace Core
