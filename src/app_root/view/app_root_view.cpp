#include "app_root/view/app_root_view.hpp"

#include <QListView>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace AppRoot {

AppRootView::AppRootView(QWidget* parent)
    : QWidget(parent),
      m_tabView(new QListView(this)),
      m_contentStack(new QStackedWidget(this)),
      m_mainLayout(new QVBoxLayout(this))
{
    // Configure the ListView as a horizontal top bar
    m_tabView->setFlow(QListView::LeftToRight);
    m_tabView->setViewMode(QListView::ListMode);
    m_tabView->setWrapping(false);
    m_tabView->setFixedHeight(50);
    m_tabView->setFrameShape(QFrame::NoFrame);
    m_tabView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tabView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Layout configuration: Top-to-Bottom
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(4);
    m_mainLayout->addWidget(m_tabView);
    m_mainLayout->addWidget(m_contentStack);

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

    // Sync content: React to model updates
    connect(m_model, &AppRootModel::rowsInserted, this, &AppRootView::onRowsInserted);
    connect(m_model, &AppRootModel::rowsAboutToBeRemoved, this,
            &AppRootView::onRowsAboutToBeRemoved);
    connect(m_model, &AppRootModel::dataChanged, this,
            [this](const QModelIndex& topLeft, const QModelIndex& bottomRight,
                   const QList<int>& roles) { this->onDataChanged(topLeft, bottomRight, roles); });
}

void AppRootView::setDelegate(AppRootDelegate* delegate) const
{
    m_tabView->setItemDelegate(delegate);
}

void AppRootView::handleTabChanged(const int index) const
{
    if (index >= 0 && index < m_contentStack->count())
    {
        m_contentStack->setCurrentIndex(index);
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
    // m_tabView->update();
}

}  // namespace AppRoot