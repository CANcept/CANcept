
#include "fault_injector_view.hpp"

#include <QHeaderView>
#include <QScrollBar>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "fault_injector/constants.hpp"
#include "fault_injector/styles.hpp"
#include "fault_injector/ui/delegate/fault_display.hpp"
#include "fault_injector/ui/delegate/fault_injector_dynamic_delegate.hpp"
#include "fault_injector/ui/delegate/fault_injector_type_delegate.hpp"
#include "fault_injector/ui/model/fault_injector_sort_proxy.hpp"

namespace FaultInjector {

FaultInjectorView::FaultInjectorView(QWidget* parent) : QWidget(parent)
{
    m_model = new FaultInjectorModel(this);

    // Corrupt CARSIDE_CONTROD (0x102) frames with 25% probability
    m_model->addFault(RawFault{
        .trigger =
            {
                IDTrigger(0x102),
                DLCTrigger(8),
                RandomTrigger(0.25f),
            },
        .effect =
            {
                BitFlipEffect(1, 4),
            },
        .strategy = ImmediateStrategy{},
    });

    // Zero out RequestedAmps on 50% of sends to simulate a charging request fault
    m_model->addFault(DbcFault{
        .trigger =
            {
                SignalNameTrigger("RequestedAmps"),
                RandomTrigger(0.5f),
            },
        .effect =
            {
                ValueSetEffect("RequestedAmps", 0.0),
            },
        .strategy = ImmediateStrategy{},
    });

    // Randomly corrupt EVSE_STATUS (0x109) frames
    m_model->addFault(RawFault{
        .trigger =
            {
                IDTrigger(0x109),
            },
        .effect =
            {
                RandomBitFlipEffect(),
            },
        .strategy = ImmediateStrategy{},
    });

    setupUi();
}

void FaultInjectorView::setupUi()
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

    m_titleLabel = new QLabel(Constants::FAULT_INJECTOR_TITLE, m_card);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(spacing.fontSizeSm);
    titleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightNormal));
    m_titleLabel->setFont(titleFont);
    textLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(Constants::FAULT_INJECTOR_SUBTITLE, m_card);
    QFont subtitleFont = m_subtitleLabel->font();
    subtitleFont.setPointSize(spacing.fontSizeXs);
    subtitleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightMedium));
    m_subtitleLabel->setFont(subtitleFont);
    textLayout->addWidget(m_subtitleLabel);

    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();

    // Toggle switch on the right
    m_toggleSwitch = new Core::StyledSwitch(m_card);
    headerLayout->addWidget(m_toggleSwitch, 0, Qt::AlignVCenter);
    cardLayout->insertLayout(0, headerLayout);

    // fault list
    m_faults = new QTableView(m_card);
    m_faults->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_faults->setVisible(false);

    // apply the sort proxy
    auto* proxy = new FaultInjectorSortProxyModel(this);
    proxy->setSourceModel(m_model);
    m_faults->setModel(proxy);
    proxy->sort(0);

    m_faults->horizontalHeader()->setSectionResizeMode(static_cast<int>(FaultListColumn::Type),
                                                       QHeaderView::Fixed);
    m_faults->horizontalHeader()->setSectionResizeMode(static_cast<int>(FaultListColumn::Triggers),
                                                       QHeaderView::Stretch);
    m_faults->horizontalHeader()->setSectionResizeMode(static_cast<int>(FaultListColumn::Effects),
                                                       QHeaderView::Stretch);
    m_faults->horizontalHeader()->setSectionResizeMode(static_cast<int>(FaultListColumn::Strategy),
                                                       QHeaderView::Fixed);
    m_faults->horizontalHeader()->hide();
    m_faults->verticalHeader()->hide();
    m_faults->setShowGrid(false);
    m_faults->setMouseTracking(true);
    m_faults->setSelectionBehavior(QAbstractItemView::SelectRows);

    // column style
    m_faults->setItemDelegateForColumn(static_cast<int>(FaultListColumn::Type),
                                       new FaultInjectorTypeDelegate(m_faults));
    m_faults->setItemDelegateForColumn(
        static_cast<int>(FaultListColumn::Triggers),
        new FaultInjectorDynamicDelegate(
            [](const QVariant& v) {
                QStringList labels;
                if (const auto raw = v.value<std::vector<RawTrigger>>(); !raw.empty())
                    for (const auto& t : raw) labels << triggerLabel(t);
                else if (const auto dbc = v.value<std::vector<DbcTrigger>>(); !dbc.empty())
                    for (const auto& t : dbc) labels << triggerLabel(t);
                return labels;
            },
            m_faults));
    m_faults->setItemDelegateForColumn(
        static_cast<int>(FaultListColumn::Effects),
        new FaultInjectorDynamicDelegate(
            [](const QVariant& v) {
                QStringList labels;
                if (const auto raw = v.value<std::vector<RawEffect>>(); !raw.empty())
                    for (const auto& e : raw) labels << effectLabel(e);
                else if (const auto dbc = v.value<std::vector<DbcEffect>>(); !dbc.empty())
                    for (const auto& e : dbc) labels << effectLabel(e);
                return labels;
            },
            m_faults));
    m_faults->setItemDelegateForColumn(
        static_cast<int>(FaultListColumn::Strategy),
        new FaultInjectorDynamicDelegate(
            [](const QVariant& v) { return QStringList{strategyLabel(v.value<Strategy>())}; },
            m_faults));

    m_tableCardWidget = new Core::CardWidget(QString(), QString(), QString(), m_card);
    m_tableCardWidget->setVisible(false);
    m_tableCardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (auto* tableCardLayout = m_tableCardWidget->contentLayout())
    {
        tableCardLayout->addWidget(m_faults);
    }
    cardLayout->addWidget(m_tableCardWidget);

    mainLayout->addWidget(m_card, 1);

    // Connect toggle signal
    connect(m_toggleSwitch, &Core::StyledSwitch::toggled, this,
            &FaultInjectorView::onToggleChanged);

    applyStyle();
}

void FaultInjectorView::applyStyle() const
{
    const auto& colors = THEME.colors();

    m_faults->setStyleSheet(QString("QTableView {"
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
                                .arg(colors.textPrimary.name())
                                .arg(colors.surfaceSelected.name())
                                .arg(colors.surfaceHover.name()));

    // Apply scrollbar style
    if (m_faults->verticalScrollBar())
        m_faults->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());

    // Apply text colors
    if (m_titleLabel)
    {
        m_titleLabel->setStyleSheet(QString("color: %1;").arg(colors.textPrimary.name()));
    }

    if (m_subtitleLabel)
    {
        m_subtitleLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    }
}

bool FaultInjectorView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void FaultInjectorView::onToggleChanged(const bool checked)
{
    m_faults->setVisible(checked);
    m_tableCardWidget->setVisible(checked);
    setMinimumHeight(checked ? maximumHeight() : 0);
}

auto FaultInjectorView::isFaultInjection() const -> bool
{
    return m_toggleSwitch->isChecked();
}

auto FaultInjectorView::getFaultHandler() const -> FaultHandler
{
    return m_model->get();
}

}  // namespace FaultInjector