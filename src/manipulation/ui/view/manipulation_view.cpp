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

#include "manipulation_view.hpp"

#include <QHeaderView>
#include <QScrollBar>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "manipulation/constants.hpp"
#include "manipulation/styles.hpp"
#include "manipulation/ui/delegate/manipulation_display.hpp"
#include "manipulation/ui/delegate/manipulation_dynamic_delegate.hpp"
#include "manipulation/ui/delegate/manipulation_type_delegate.hpp"
#include "manipulation/ui/model/manipulation_sort_proxy.hpp"

namespace Manipulation {

ManipulationView::ManipulationView(QWidget* parent)
    : QWidget(parent), m_model(new ManipulationModel(this)), m_dialog(new ManipulationDialog(this))
{
    setupUi();
}

void ManipulationView::setupUi()
{
    const auto& spacing = THEME.spacing();

    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_card = new Core::CardWidget(QString(), QString(), QString(), this);
    auto* cardLayout = m_card->contentLayout();

    // Header
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(spacing.spacingSm);

    // Title and subtitle
    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(spacing.spacingXs);
    textLayout->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new QLabel(Constants::MANIPULATION_TITLE, m_card);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(spacing.fontSizeSm);
    titleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightNormal));
    m_titleLabel->setFont(titleFont);
    textLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(Constants::MANIPULATION_SUBTITLE, m_card);
    QFont subtitleFont = m_subtitleLabel->font();
    subtitleFont.setPointSize(spacing.fontSizeXs);
    subtitleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightMedium));
    m_subtitleLabel->setFont(subtitleFont);
    textLayout->addWidget(m_subtitleLabel);

    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();

    // Add manipulation buttons
    m_addRawButton = new QPushButton(Constants::MANIPULATION_ADD_RAW_BUTTON_LABEL, m_card);
    m_addRawButton->setVisible(false);
    headerLayout->addWidget(m_addRawButton, 0, Qt::AlignVCenter);

    m_addDbcButton = new QPushButton(Constants::MANIPULATION_ADD_DBC_BUTTON_LABEL, m_card);
    m_addDbcButton->setVisible(false);
    headerLayout->addWidget(m_addDbcButton, 0, Qt::AlignVCenter);

    // Toggle switch on the right
    m_toggleSwitch = new Core::StyledSwitch(m_card);
    headerLayout->addWidget(m_toggleSwitch, 0, Qt::AlignVCenter);
    cardLayout->insertLayout(0, headerLayout);

    // manipulation list
    m_manipulations = new QTableView(m_card);
    m_manipulations->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_manipulations->setVisible(false);

    // apply the sort proxy
    auto* proxy = new ManipulationSortProxyModel(this);
    proxy->setSourceModel(m_model);
    m_manipulations->setModel(proxy);
    proxy->sort(0);

    m_manipulations->horizontalHeader()->setSectionResizeMode(
        static_cast<int>(ManipulationListColumn::Type), QHeaderView::Fixed);
    m_manipulations->horizontalHeader()->setSectionResizeMode(
        static_cast<int>(ManipulationListColumn::Triggers), QHeaderView::Stretch);
    m_manipulations->horizontalHeader()->setSectionResizeMode(
        static_cast<int>(ManipulationListColumn::Effects), QHeaderView::Stretch);
    m_manipulations->horizontalHeader()->setSectionResizeMode(
        static_cast<int>(ManipulationListColumn::Strategy), QHeaderView::Fixed);
    m_manipulations->horizontalHeader()->hide();
    m_manipulations->verticalHeader()->hide();
    m_manipulations->setShowGrid(false);
    m_manipulations->setMouseTracking(true);
    m_manipulations->setSelectionBehavior(QAbstractItemView::SelectRows);

    // column style
    m_manipulations->setItemDelegateForColumn(static_cast<int>(ManipulationListColumn::Type),
                                              new ManipulationTypeDelegate(m_manipulations));
    m_manipulations->setItemDelegateForColumn(
        static_cast<int>(ManipulationListColumn::Triggers),
        new ManipulationDynamicDelegate(
            [](const QVariant& v) -> QStringList {
                QStringList labels;
                if (const auto raw = v.value<std::vector<RawTrigger>>(); !raw.empty())
                    for (const auto& t : raw) labels << triggerLabel(t);
                else if (const auto dbc = v.value<std::vector<DbcTrigger>>(); !dbc.empty())
                    for (const auto& t : dbc) labels << triggerLabel(t);
                return labels;
            },
            m_manipulations));
    m_manipulations->setItemDelegateForColumn(
        static_cast<int>(ManipulationListColumn::Effects),
        new ManipulationDynamicDelegate(
            [](const QVariant& v) -> QStringList {
                QStringList labels;
                if (const auto raw = v.value<std::vector<RawEffect>>(); !raw.empty())
                    for (const auto& e : raw) labels << effectLabel(e);
                else if (const auto dbc = v.value<std::vector<DbcEffect>>(); !dbc.empty())
                    for (const auto& e : dbc) labels << effectLabel(e);
                return labels;
            },
            m_manipulations));
    m_manipulations->setItemDelegateForColumn(
        static_cast<int>(ManipulationListColumn::Strategy),
        new ManipulationDynamicDelegate(
            [](const QVariant& v) -> QStringList {
                return QStringList{strategyLabel(v.value<Strategy>())};
            },
            m_manipulations));
    m_manipulations->setItemDelegateForColumn(
        static_cast<int>(ManipulationListColumn::Mutation),
        new ManipulationDynamicDelegate(
            [](const QVariant& v) -> QStringList {
                return QStringList{mutationLabel(v.value<Mutation>())};
            },
            m_manipulations));

    m_tableCardWidget = new Core::CardWidget(QString(), QString(), QString(), m_card);
    m_tableCardWidget->setVisible(false);
    m_tableCardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (auto* tableCardLayout = m_tableCardWidget->contentLayout())
    {
        tableCardLayout->addWidget(m_manipulations);
    }
    cardLayout->addWidget(m_tableCardWidget);

    mainLayout->addWidget(m_card, 1);

    // Connect toggle signal
    connect(m_toggleSwitch, &Core::StyledSwitch::toggled, this, &ManipulationView::onToggleChanged);

    connect(m_addRawButton, &QPushButton::clicked, this, &ManipulationView::onAddRawClicked);
    connect(m_addDbcButton, &QPushButton::clicked, this, &ManipulationView::onAddDbcClicked);
    connect(m_manipulations, &QTableView::clicked, this, &ManipulationView::onManipulationClicked);

    applyStyle();
}

void ManipulationView::applyStyle() const
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    m_manipulations->setStyleSheet(QString("QTableView {"
                                           "  background-color: transparent;"
                                           "  border: none;"
                                           "  outline: none;"
                                           "}"
                                           "QTableView::item {"
                                           "  border: none;"
                                           "  color: %1;"
                                           "}"
                                           "QTableView::item:selected {"
                                           "  background-color: %2;"
                                           "  color: %1;"
                                           "}"
                                           "QTableView::item:hover {"
                                           "  background-color: %3;"
                                           "}")
                                       .arg(colors.textPrimary.name(),
                                            colors.surfaceSelected.name(),
                                            colors.surfaceHover.name()));

    // Apply scrollbar style
    if (m_manipulations->verticalScrollBar())
        m_manipulations->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());

    // Apply text colors
    if (m_titleLabel)
    {
        m_titleLabel->setStyleSheet(QString("color: %1;").arg(colors.textPrimary.name()));
    }

    if (m_subtitleLabel)
    {
        m_subtitleLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    }

    const QString buttonStyle = QString(
                                    "QPushButton {"
                                    "   border: none;"
                                    "   border-radius: %1px;"
                                    "   font-size: %2px;"
                                    "   font-weight: %3;"
                                    "   padding: %4px %5px;"
                                    "   background-color: %6;"
                                    "   color: %7;"
                                    "}"
                                    "QPushButton:hover {"
                                    "   background-color: %8;"
                                    "}"
                                    "QPushButton:pressed {"
                                    "   background-color: %8;"
                                    "}")
                                    .arg(spacing.radiusSm)
                                    .arg(spacing.fontSizeMd)
                                    .arg(spacing.fontWeightMedium)
                                    .arg(spacing.spacingXs)
                                    .arg(spacing.spacingLg)
                                    .arg(colors.surfacePrimary.name(), colors.textPrimary.name(),
                                         colors.colorPrimaryHover.name());

    if (m_addRawButton)
    {
        m_addRawButton->setStyleSheet(buttonStyle);
    }

    if (m_addDbcButton)
    {
        m_addDbcButton->setStyleSheet(buttonStyle);
    }
}

auto ManipulationView::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void ManipulationView::onAddRawClicked() const
{
    if (m_dialog->open(true) != QDialog::Accepted)
    {
        return;
    }

    if (const auto manipulation = m_dialog->acquire())
    {
        m_model->addManipulation(*manipulation);
    }
}

void ManipulationView::onAddDbcClicked() const
{
    if (m_dialog->open(false) != QDialog::Accepted)
    {
        return;
    }

    if (const auto manipulation = m_dialog->acquire())
    {
        m_model->addManipulation(*manipulation);
    }
}

void ManipulationView::onToggleChanged(const bool checked)
{
    m_manipulations->setVisible(checked);
    m_tableCardWidget->setVisible(checked);
    m_addRawButton->setVisible(checked);
    m_addDbcButton->setVisible(checked && m_model->mode() == ManipulationModel::Mode::Dbc);
    setMinimumHeight(checked ? maximumHeight() : 0);
}

void ManipulationView::setMode(const ManipulationModel::Mode mode)
{
    m_model->setMode(mode);
    // Sync button visibility if the toggle is currently on.
    if (m_toggleSwitch->isChecked())
    {
        m_addDbcButton->setVisible(mode == ManipulationModel::Mode::Dbc);
    }
}

void ManipulationView::onManipulationClicked(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return;
    }
    const auto* proxy = qobject_cast<const QSortFilterProxyModel*>(m_manipulations->model());
    const int sourceRow = proxy ? proxy->mapToSource(index).row() : index.row();
    m_model->removeManipulation(sourceRow);
}

auto ManipulationView::isManipulation() const -> bool
{
    return m_toggleSwitch->isChecked();
}

auto ManipulationView::getManipulationHandler() const -> ManipulationHandler
{
    return m_model->get();
}

}  // namespace Manipulation