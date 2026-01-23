#include "sending_view.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

SendingView::SendingView(QWidget* parent)
    : QWidget(parent),
      m_btnRawMode(nullptr),
      m_btnDbcMode(nullptr),
      m_contentStack(nullptr),
      m_rawView(nullptr),
      m_dbcView(nullptr)
{
    setupUi();
}

void SendingView::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === Sidebar ===
    auto* sidebar = new QWidget(this);
    sidebar->setFixedWidth(120);
    sidebar->setStyleSheet(QString("QWidget { background-color: %1; }"
                                   "QPushButton { "
                                   "  text-align: left; "
                                   "  padding: %2px %3px; "
                                   "  border: none; "
                                   "  background-color: transparent; "
                                   "  font-size: %4px; "
                                   "}"
                                   "QPushButton:hover { "
                                   "  background-color: %5; "
                                   "}"
                                   "QPushButton:checked { "
                                   "  background-color: %6; "
                                   "  font-weight: %7; "
                                   "  border-left: %8px solid %9; "
                                   "}")
                               .arg(colors.surfacePrimary.name())
                               .arg(spacing.spacingMd)
                               .arg(spacing.spacingLg)
                               .arg(spacing.fontSizeSm)
                               .arg(colors.surfaceSecondary.name())
                               .arg(colors.surfaceMain.name())
                               .arg(spacing.fontWeightBold)
                               .arg(spacing.borderThick + 1)
                               .arg(colors.textSecondary.name()));

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, spacing.spacingLg, 0, spacing.spacingLg);
    sidebarLayout->setSpacing(spacing.spacingXs);

    // Raw mode button
    m_btnRawMode = new QPushButton(tr("Raw"), sidebar);
    m_btnRawMode->setCheckable(true);
    m_btnRawMode->setChecked(true);  // Default to raw mode
    m_btnRawMode->setIcon(QIcon(Constants::RAW_SENDING_ICON_PATH));

    // DBC-Based mode button
    m_btnDbcMode = new QPushButton(tr("DBC-Based"), sidebar);
    m_btnDbcMode->setCheckable(true);
    m_btnDbcMode->setIcon(QIcon(Constants::DBC_SENDING_ICON_PATH));

    sidebarLayout->addWidget(m_btnRawMode);
    sidebarLayout->addWidget(m_btnDbcMode);
    sidebarLayout->addStretch();

    mainLayout->addWidget(sidebar);

    // === Content Stack ===
    m_contentStack = new QStackedWidget(this);

    // Create sub-views
    m_rawView = new RawSendingSubView(m_contentStack);
    m_dbcView = new DbcSendingSubView(m_contentStack);

    m_contentStack->addWidget(m_rawView);  // Index 0
    m_contentStack->addWidget(m_dbcView);  // Index 1

    mainLayout->addWidget(m_contentStack, 1);

    // === Connect sidebar buttons ===
    connect(m_btnRawMode, &QPushButton::clicked, this, [this]() { displayMode(0); });

    connect(m_btnDbcMode, &QPushButton::clicked, this, [this]() { displayMode(1); });
}

void SendingView::displayMode(int index)
{
    m_contentStack->setCurrentIndex(index);

    // Update button states
    m_btnRawMode->setChecked(index == 0);
    m_btnDbcMode->setChecked(index == 1);

    emit modeChanged(index == 1);
}

void SendingView::setModel(SendingModel* /*model*/)
{
    // Model binding can be implemented here if needed
    // For now, the component handles the connections directly
}

void SendingView::setAvailableDevices(const std::vector<std::string>& devices)
{
    if (m_rawView)
    {
        m_rawView->setAvailableInterfaces(devices);
    }
    if (m_dbcView)
    {
        m_dbcView->setAvailableInterfaces(devices);
    }
}

void SendingView::setAvailableSpeeds(const std::vector<uint32_t>& speeds)
{
    // Set baud rates for raw view
    if (m_rawView)
    {
        m_rawView->setAvailableBaudRates(speeds);
    }
    // DBC view doesn't have baud rate selection (uses global configuration)
}

}  // namespace Sending
