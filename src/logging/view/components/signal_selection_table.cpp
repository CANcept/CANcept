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

#include "signal_selection_table.hpp"

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Logging {

// Constructs a signal selection widget for a specific CAN message
SignalSelectionTree::SignalSelectionTree(uint32_t messageId, const QString& messageName,
                                         QWidget* parent)
    : QWidget(parent),
      m_messageId(messageId),
      m_messageName(messageName),
      m_signalList(nullptr),
      m_signalModel(nullptr),
      m_selectAllCheckbox(nullptr)
{
    setupUi();
}

// Initializes the UI with checkbox and signal list view
void SignalSelectionTree::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    // Header with "Select All" checkbox
    auto* headerLayout = new QHBoxLayout();

    m_selectAllCheckbox = new QCheckBox("Select All Signals", this);

    connect(m_selectAllCheckbox, &QCheckBox::toggled, this,
            [this](bool checked) { setAllSignalsSelected(checked); });

    headerLayout->addWidget(m_selectAllCheckbox);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    // Signal list view
    m_signalList = new QListView(this);
    m_signalList->setViewMode(QListView::ListMode);
    m_signalList->setResizeMode(QListView::Adjust);
    m_signalList->setWrapping(false);
    m_signalList->setUniformItemSizes(true);
    m_signalList->setMaximumHeight(200);

    m_signalModel = new QStandardItemModel(this);
    m_signalList->setModel(m_signalModel);

    layout->addWidget(m_signalList);
    applyStyle();
}

// Populates the list with signals from DBC message description
void SignalSelectionTree::setSignals(const std::list<Core::DbcSignalDescription>& signalList)
{
    m_signalModel->clear();

    for (const auto& signal : signalList)
    {
        auto* item = new QStandardItem();

        // Create signal display text with name, unit, and range
        QString displayText = QString::fromStdString(signal.signalName);
        if (!signal.unit.empty())
        {
            displayText += QString(" [%1]").arg(QString::fromStdString(signal.unit));
        }
        displayText += QString(" (%.1f - %.1f)").arg(signal.minimum).arg(signal.maximum);

        item->setText(displayText);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);  // Default: all signals selected

        // Store the signal name in UserRole for easy retrieval
        item->setData(QString::fromStdString(signal.signalName), Qt::UserRole + 1);

        m_signalModel->appendRow(item);
    }

    // Update "Select All" checkbox state
    m_selectAllCheckbox->setChecked(m_signalModel->rowCount() > 0);

    // Connect model changes to emit selection changed signal
    connect(m_signalModel, &QStandardItemModel::itemChanged, this, [this](QStandardItem* item) {
        Q_UNUSED(item);
        int selectedCount = getSelectedSignals().count();
        emit selectionChanged(m_messageId, selectedCount);

        // Update "Select All" checkbox state without triggering its signal
        m_selectAllCheckbox->blockSignals(true);
        m_selectAllCheckbox->setChecked(selectedCount == m_signalModel->rowCount());
        m_selectAllCheckbox->setTristate(selectedCount > 0 &&
                                         selectedCount < m_signalModel->rowCount());
        if (selectedCount > 0 && selectedCount < m_signalModel->rowCount())
        {
            m_selectAllCheckbox->setCheckState(Qt::PartiallyChecked);
        }
        m_selectAllCheckbox->blockSignals(false);
    });
}

// Returns list of signal names that are currently checked
QStringList SignalSelectionTree::getSelectedSignals() const
{
    QStringList selectedSignals;

    for (int i = 0; i < m_signalModel->rowCount(); ++i)
    {
        QStandardItem* item = m_signalModel->item(i);
        if (item && item->checkState() == Qt::Checked)
        {
            QString signalName = item->data(Qt::UserRole + 1).toString();
            selectedSignals.append(signalName);
        }
    }

    return selectedSignals;
}

// Selects or deselects all signals in the list
void SignalSelectionTree::setAllSignalsSelected(bool checked)
{
    m_signalModel->blockSignals(true);

    for (int i = 0; i < m_signalModel->rowCount(); ++i)
    {
        QStandardItem* item = m_signalModel->item(i);
        if (item)
        {
            item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        }
    }

    m_signalModel->blockSignals(false);

    // Emit signal once after all changes
    int selectedCount = checked ? m_signalModel->rowCount() : 0;
    emit selectionChanged(m_messageId, selectedCount);
}

void SignalSelectionTree::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    if (m_selectAllCheckbox)
    {
        m_selectAllCheckbox->setStyleSheet(QString("QCheckBox {"
                                                   "   font-size: %1px;"
                                                   "   font-weight: %2;"
                                                   "   color: %3;"
                                                   "}")
                                               .arg(spacing.fontSizeMd)
                                               .arg(spacing.fontWeightMedium)
                                               .arg(colors.textPrimary.name()));
    }
    if (m_signalList)
    {
        m_signalList->setStyleSheet(QString("QListView {"
                                            "   background-color: %1;"
                                            "   border: %2px solid %3;"
                                            "   border-radius: %4px;"
                                            "   padding: %5px;"
                                            "   font-size: %6px;"
                                            "}"
                                            "QListView::item {"
                                            "   padding: %5px;"
                                            "   border-radius: %7px;"
                                            "}"
                                            "QListView::item:hover {"
                                            "   background-color: %8;"
                                            "}")
                                        .arg(colors.surfacePrimary.name())
                                        .arg(spacing.borderThin)
                                        .arg(colors.borderSubtle.name())
                                        .arg(spacing.radiusSm)
                                        .arg(spacing.spacingXs)
                                        .arg(spacing.fontSizeMd)
                                        .arg(spacing.radiusXs)
                                        .arg(colors.surfaceHover.name()));
    }
}

bool SignalSelectionTree::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace Logging
