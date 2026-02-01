#include "monitoring_view.hpp"

#include <QHeaderView>
#include <QTimer>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "graph_list_view.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

MonitoringView::MonitoringView(MonitoringModel* model, MonitoringDelegate* delegate)
    : QWidget(nullptr),
      m_treeProxy(new QSortFilterProxyModel(this)),
      m_signalListView(new SignalList(this)),
      m_splitter(new QSplitter(Qt::Horizontal, this)),
      m_dbcMessageTimer(new QTimer(this)),
      m_rawMessageTimer(new QTimer(this))
{
    m_model = model;
    m_delegate = delegate;
    setupUi();
}

void MonitoringView::setupUi()
{
    m_graphListView = new GraphListView(m_model, m_delegate);
    m_signalListView->setModel(m_model);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    mainLayout->setSpacing(spacing.spacingLg);

    // --- CAN-Bus Connection Group ---
    m_connectionGroup = new QGroupBox(this);  // No title here, we'll add a custom one
    m_connectionGroup->setStyleSheet(
        "QGroupBox { border: 1px solid #C0C0C0; border-radius: 8px; }");
    auto* groupLayout = new QVBoxLayout(m_connectionGroup);

    // --- Row 1: Config & Connect ---
    auto* topRow = new QHBoxLayout();

    // Title with Icon
    m_titleIcon = new QLabel(this);
    m_titleIcon->setPixmap(
        QPixmap(Core::Assets::MonitoringBusConfigIconPath).scaled(24, 24, Qt::KeepAspectRatio));
    auto* titleLabel = new QLabel("CAN-Bus Connection", this);
    titleLabel->setStyleSheet("font-size: 20px;");

    m_interfaceCombo = new QComboBox(this);
    m_interfaceCombo->setStyleSheet(
        QString("background-color: %1; width: 300px; border-radius: 15px; font-size: 15px;")
            .arg(THEME.colors().surfacePrimary.name()));
    m_interfaceCombo->addItems({"vcan0", "can0", "can1"});

    m_connectButton = new QPushButton("Connect", this);
    m_connectButton->setIcon(QIcon(Core::Assets::MonitoringBusDisconPath));  // Initial state
    m_connectButton->setStyleSheet(
        QString("background-color: %1; width: 100px; border-radius: 15px; font-size: 15px;")
            .arg(THEME.colors().surfacePrimary.name()));

    topRow->addWidget(m_titleIcon);
    topRow->addWidget(titleLabel);
    topRow->addStretch();
    topRow->addSpacing(30);
    topRow->addWidget(m_interfaceCombo);
    topRow->addSpacing(30);
    topRow->addWidget(m_connectButton);

    // --- Row 2: Status Boxes ---
    auto* bottomRow = new QHBoxLayout();

    // Create the three boxes
    QFrame* statusBox = createStatBox("Status", m_statusValueLabel = new QLabel("Disconnected"));
    QFrame* fpsBox = createStatBox("Frame rate", m_fpsValueLabel = new QLabel("0 fps"));
    QFrame* msgBox = createStatBox("Messages", m_msgCountValueLabel = new QLabel("0 messages"));

    // Set initial status color
    m_statusValueLabel->setStyleSheet("font-size: 15px; color: red;");
    m_fpsValueLabel->setStyleSheet("font-size: 15px; color: gray;");
    m_msgCountValueLabel->setStyleSheet("font-size: 15px; color: gray;");

    bottomRow->addWidget(statusBox, 1);  // The '1' makes them share space equally
    bottomRow->addWidget(fpsBox, 1);
    bottomRow->addWidget(msgBox, 1);

    // Add rows to group
    groupLayout->addLayout(topRow);
    groupLayout->addLayout(bottomRow);

    // Configure SignalList
    m_signalListView->setStyleSheet("QTreeView { border: none; background-color: transparent;");

    // Configure Splitter
    m_splitter->addWidget(m_signalListView);
    m_splitter->addWidget(m_graphListView);

    // Give the Graph view more initial space (e.g., 30/70 split)
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);

    // --- Add to Main Layout ---
    mainLayout->addWidget(m_connectionGroup, 1);
    mainLayout->addWidget(m_splitter, 5);

    // Connect the data changed signal of the model to intercept checkbox toggles
    // We do this via the proxy so we get the correct indices
    connect(m_treeProxy, &QSortFilterProxyModel::dataChanged, this,
            [this](const QModelIndex& topLeft, const QModelIndex& bottomRight,
                   const QVector<int>& roles) -> void {
                if (roles.contains(Qt::CheckStateRole) || roles.isEmpty())
                {
                    // In a tree, we check if the item now has a checkmark
                    bool isChecked = topLeft.data(Qt::CheckStateRole) == Qt::Checked;

                    // Extract data needed for the signals
                    // Note: We assume the model stores ID and Name in specific user roles or
                    // columns
                    char msgId = static_cast<char>(topLeft.data(Qt::UserRole + 1).toInt());
                    std::string sigName = topLeft.data(Qt::DisplayRole).toString().toStdString();

                    if (isChecked)
                    {
                        emit signalChecked(msgId, sigName);
                    } else
                    {
                        emit signalUnchecked(msgId, sigName);
                    }
                }
            });

    connect(m_connectButton, &QPushButton::clicked, this, [this]() -> void {
        if (m_connectButton->text() == "Connect")
        {
            // Switch to Connected state
            m_connectButton->setText("Disconnect");
            m_connectButton->setIcon(QIcon(Core::Assets::MonitoringBusDisconPath));
            m_connectButton->setStyleSheet(QString("background-color: %1; color: black; width: "
                                                   "100px; border-radius: 15px; font-size: 15px;")
                                               .arg(THEME.colors().statusWarning.name()));

            m_statusValueLabel->setText(
                QString("Connected (%1)").arg(m_interfaceCombo->currentText()));
            m_statusValueLabel->setStyleSheet("font-size: 15px; color: green;");
        } else
        {
            // Switch to Disconnected state
            m_connectButton->setText("Connect");
            m_connectButton->setIcon(QIcon(Core::Assets::MonitoringBusConnPath));
            m_connectButton->setStyleSheet(QString("background-color: %1; color: black; width: "
                                                   "100px; border-radius: 15px; font-size: 15px;")
                                               .arg(THEME.colors().surfacePrimary.name()));

            m_statusValueLabel->setText("Disconnected");
            m_statusValueLabel->setStyleSheet("font-size:15px; color: red;");
        }
    });
}

// Helper function to keep code clean
QFrame* MonitoringView::createStatBox(const QString& title, QLabel*& valueLabel)
{
    auto* frame = new QFrame(this);
    frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    frame->setFrameStyle(QFrame::StyledPanel);
    frame->setStyleSheet(
        "QFrame { border: 1px solid #C0C0C0; border-radius: 8px; }"
        "QLabel { border: none; background: transparent; }");

    auto* layout = new QVBoxLayout(frame);

    layout->setContentsMargins(20, 5, 5, 10);
    layout->setSpacing(5);

    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px;");

    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return frame;
}

void MonitoringView::onDbcConfigurationChanged()
{
    // If the DBC changes, the old graphs are likely invalid
    if (m_graphListView)
    {
        // Assuming GraphListView has a clear or reset method
        // If not, we trigger the logic here
    }

    // Refresh the tree view
    m_signalListView->clearMessages();
}
}  // namespace Monitoring
