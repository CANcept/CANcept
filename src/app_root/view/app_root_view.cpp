#include "app_root/view/app_root_view.hpp"

#include <qevent.h>

#include <QEvent>
#include <QListView>
#include <QPainter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "app_root/constants.hpp"
#include "core/macro/console_logging.hpp"
#include "core/macro/theme.hpp"

namespace AppRoot {

AppRootView::AppRootView(QWidget* parent)
    : QWidget(parent),
      m_tabView(new QListView(this)),
      m_contentStack(new QStackedWidget(this)),
      m_mainLayout(new QVBoxLayout(this)),
      m_topBarLayout(new QHBoxLayout()),
      m_logoLabel(new QLabel(this))
{
    // Set Initial Window Size
    resize(1200, 800);

    // Set Background Color
    this->setObjectName("AppRootView");
    this->setStyleSheet(QString("#AppRootView {"
                                "  background-color: %1;"
                                "}")
                            .arg(THEME.colors().surfaceMain.name()));

    // Configure the ListView as a horizontal top bar
    m_tabView->setFlow(QListView::LeftToRight);
    m_tabView->setViewMode(QListView::ListMode);
    m_tabView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_tabView->setWrapping(false);
    m_tabView->setUniformItemSizes(true);
    m_tabView->setFixedHeight(THEME.spacing().radiusMd * 2);
    m_tabView->setFrameShape(QFrame::NoFrame);
    m_tabView->setFocusPolicy(Qt::StrongFocus);
    m_tabView->setAttribute(Qt::WA_StyledBackground);
    m_tabView->setMouseTracking(true);
    m_tabView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tabView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tabView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabView->viewport()->installEventFilter(this);
    m_tabView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tabView->setStyleSheet(QString("QListView {"
                                     "background-color: %1;"
                                     "border-radius: %2px;"
                                     "border: none;"
                                     "outline: none;"
                                     "}"
                                     "")
                                 .arg(THEME.colors().surfacePrimary.name())
                                 .arg(THEME.spacing().radiusMd));

    // Layout configuration: Top-to-Bottom
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(THEME.spacing().spacingXs);
    m_mainLayout->addLayout(m_topBarLayout);
    m_mainLayout->addWidget(m_contentStack);

    // LAyout Configuration: Left-to-Right
    m_topBarLayout->setContentsMargins(THEME.spacing().spacingLg, THEME.spacing().spacingMd,
                                       THEME.spacing().spacingXl, THEME.spacing().spacingMd);
    m_topBarLayout->setSpacing(THEME.spacing().spacingXl);
    m_topBarLayout->addWidget(m_logoLabel);
    m_topBarLayout->addWidget(m_tabView, 1);
    m_topBarLayout->addStretch();

    // Render CanBusManager Icon on the left
    constexpr QSize iconSize(22, 26);
    const QIcon icon(Constants::CAN_BUS_ICON_PATH);
    QPixmap pixmap = icon.pixmap(iconSize, devicePixelRatioF());

    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), THEME.colors().surfaceForeground);
    painter.end();

    m_logoLabel->setPixmap(pixmap);
    m_logoLabel->setFixedSize(iconSize);

    setLayout(m_mainLayout);
}

void AppRootView::setModel(AppRootModel* model)
{
    m_model = model;
    m_tabView->setModel(m_model);

    if (m_model->rowCount() > 0)
    {
        onRowsInserted(QModelIndex(), 0, m_model->rowCount() - 1);
    }

    // Switch tabs on selection
    connect(m_tabView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                handleTabChanged(current.row());
            });

    // Disable clicking on no tab atall
    connect(m_tabView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
                m_tabView->viewport()->update();

                if (selected.isEmpty() && !deselected.isEmpty() && m_model->rowCount() > 0)
                {
                    const QModelIndex lastIndex = deselected.first().indexes().first();
                    m_tabView->selectionModel()->select(lastIndex, QItemSelectionModel::Select);
                }
            });

    // Sync content: React to model updates
    connect(m_model, &AppRootModel::rowsInserted, this, &AppRootView::onRowsInserted);
    connect(m_model, &AppRootModel::rowsAboutToBeRemoved, this,
            &AppRootView::onRowsAboutToBeRemoved);
    connect(m_model, &AppRootModel::dataChanged, this, &AppRootView::onDataChanged);

    // Initialize the UI with the first tab selected
    if (m_model->rowCount() > 0)
    {
        const QModelIndex firstIndex = m_model->index(0, 0);
        m_tabView->selectionModel()->setCurrentIndex(
            firstIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

void AppRootView::setDelegate(AppRootDelegate* delegate) const
{
    m_tabView->setItemDelegate(delegate);
}

void AppRootView::handleTabChanged(const int index) const
{
    LOG_INF("AppRoot", "Switching tab...")
    if (index >= 0 && index < m_contentStack->count())
    {
        m_contentStack->setCurrentIndex(index);
    }
}

bool AppRootView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_tabView->viewport() && event->type() == QEvent::MouseButtonPress)
    {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (const QModelIndex index = m_tabView->indexAt(mouseEvent->pos()); !index.isValid())
        {
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AppRootView::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (m_model && m_tabView->selectionModel() &&
        !m_tabView->selectionModel()->currentIndex().isValid())
    {
        if (m_model->rowCount() > 0)
        {
            const QModelIndex firstIndex = m_model->index(0, 0);
            m_tabView->selectionModel()->setCurrentIndex(
                firstIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            handleTabChanged(0);
        }
    }
}

void AppRootView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_model)
    {
        emit m_model->layoutChanged();
    }
}

void AppRootView::onRowsInserted(const QModelIndex&, const int first, const int last) const
{
    for (int i = first; i <= last; ++i)
    {
        // Retrieve component from model to extract its widget
        if (auto* component = m_model->componentAt(i); component && component->getView())
        {
            m_contentStack->insertWidget(i, component->getView());
        }
    }
}

void AppRootView::onRowsAboutToBeRemoved(const QModelIndex&, int first, int last) const
{
    for (int i = last; i >= first; --i)
    {
        if (QWidget* widget = m_contentStack->widget(i))
        {
            m_contentStack->removeWidget(widget);
        }
    }
}
void AppRootView::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                const QList<int>& roles) const
{
    m_tabView->update();
}

}  // namespace AppRoot