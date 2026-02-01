#include "sending_view.hpp"

#include <QHBoxLayout>
#include <QList>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

SendingView::SendingView(QWidget* parent)
    : QWidget(parent),
      m_sidebarList(nullptr),
      m_contentStack(nullptr),
      m_rawView(nullptr),
      m_dbcView(nullptr)
{
    setupUi();
}

void SendingView::disableSidebarDeselection()
{
    // Get selection model
    auto* selectionModel = m_sidebarList->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
            [selectionModel](const QItemSelection& selected, const QItemSelection& deselected) {
                if (selected.indexes().isEmpty())
                {
                    // Reselect previous selection again
                    if (!deselected.indexes().isEmpty())
                    {
                        selectionModel->select(
                            deselected, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                        selectionModel->setCurrentIndex(deselected.indexes().first(),
                                                        QItemSelectionModel::NoUpdate);
                    }
                }
            });
}

void SendingView::setupSidebarList()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    m_sidebarList = new QListView(this);
    m_sidebarList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sidebarList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sidebarList->setMaximumWidth(200);
    m_sidebarList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_sidebarList->setFrameShape(QFrame::NoFrame);
    m_sidebarList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_sidebarList->setSelectionRectVisible(false);
    m_sidebarList->setStyleSheet(QString(R"(
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
                                     .arg(spacing.borderThick)
                                     .arg(colors.borderSubtle.name(QColor::HexArgb))
                                     .arg(colors.textSecondary.name(QColor::HexArgb))
                                     .arg(spacing.fontSizeMd)
                                     .arg(spacing.radiusSm)
                                     .arg(spacing.spacingXl)
                                     .arg(spacing.spacingMd)
                                     .arg(colors.surfacePrimary.name(QColor::HexArgb))
                                     .arg(colors.textPrimary.name(QColor::HexArgb)));
}

void SendingView::setSidebarModel()
{
    auto* sidebarModel = new QStandardItemModel(this);
    const QList<SidebarEntry> sidebarEntries = {
        {.iconPath = Constants::RAW_SENDING_ICON_PATH,
         .title = Constants::RAW_MODE_BUTTON_TEXT,
         .enabled = true},
        {.iconPath = Constants::DBC_SENDING_ICON_PATH,
         .title = Constants::DBC_MODE_BUTTON_TEXT,
         .enabled = true},
    };

    for (const auto& [iconPath, title, enabled] : sidebarEntries)
    {
        auto* item = new QStandardItem(QIcon(iconPath), title);
        item->setEnabled(enabled);
        item->setSelectable(enabled);
        sidebarModel->appendRow(item);
    }

    m_sidebarList->setModel(sidebarModel);
    disableSidebarDeselection();
    const QModelIndex firstIndex = sidebarModel->index(0, 0);
    m_sidebarList->setCurrentIndex(firstIndex);
}

void SendingView::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupSidebarList();
    setSidebarModel();
    mainLayout->addWidget(m_sidebarList);

    m_contentStack = new QStackedWidget(this);

    // Create sub-views
    m_rawView = new RawSendingSubView(m_contentStack);
    m_dbcView = new DbcSendingSubView(m_contentStack);

    m_contentStack->addWidget(m_rawView);  // Index 0
    m_contentStack->addWidget(m_dbcView);  // Index 1

    mainLayout->addWidget(m_contentStack, 1);

    connect(m_sidebarList, &QListView::clicked, this, &SendingView::onSidebarSelectionChanged);
}

void SendingView::onSidebarSelectionChanged(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    if (!(index.flags() & Qt::ItemIsEnabled))
    {
        return;
    }
    displayMode(index.row());
}

void SendingView::displayMode(const int index)
{
    m_contentStack->setCurrentIndex(index);
    emit modeChanged(index == 1);
}

void SendingView::setModel(SendingModel* model)
{
    if (!model)
    {
        return;
    }

    m_model = model;

    // Mode Changes
    connect(this, &SendingView::modeChanged, model, [model](const bool isDbcMode) {
        const auto mode = isDbcMode ? SendingModel::Mode::Dbc : SendingModel::Mode::Raw;
        model->setData(QModelIndex(), static_cast<int>(mode), SendingModel::Role_ActiveMode);
    });

    // DBC View: Message/Signal Selection
    connect(m_dbcView, &DbcSendingSubView::messageSelectionChanged, this,
            [this, model](uint16_t messageId, const bool selected) {
                model->setMessageSelected(messageId, selected);
                updateSendButtonStates();
            });

    connect(m_dbcView, &DbcSendingSubView::signalSelectionChanged, this,
            [this, model](uint16_t messageId, const QString& signalName, const bool selected) {
                model->setSignalSelected(messageId, signalName.toStdString(), selected);
                updateSendButtonStates();
            });

    // DBC View: Signal Value Changes to Model
    connect(m_dbcView, &DbcSendingSubView::signalValueChanged, model,
            [model](uint16_t messageId, const QString& signalName, const double newValue) {
                model->setSignalValue(messageId, signalName.toStdString(), newValue);
            });

    // Interface Selection to Emit Signal
    connect(m_rawView->interfaceSelector(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](const int index) {
                m_rawInterfaceSelected = (index >= 0);
                if (index >= 0)
                {
                    emit deviceSelectionChanged(
                        m_rawView->interfaceSelector()->currentText().toStdString());
                }
                updateSendButtonStates();
            });

    connect(m_dbcView->interfaceSelector(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](const int index) {
                m_dbcInterfaceSelected = (index >= 0);
                if (index >= 0)
                {
                    emit deviceSelectionChanged(
                        m_dbcView->interfaceSelector()->currentText().toStdString());
                }
                updateSendButtonStates();
            });

    // Raw CAN ID input changes to Model
    connect(m_rawView->canIdEditor(), &QLineEdit::textChanged, this, [model](const QString& text) {
        QString canIdText = text.trimmed();
        if (canIdText.startsWith("0x", Qt::CaseInsensitive))
        {
            canIdText = canIdText.mid(2);
        }
        canIdText = canIdText.remove(' ');

        bool ok = false;
        const uint16_t canId = canIdText.toUInt(&ok, 16);
        if (ok)
        {
            model->setRawCanId(canId);
        }
    });

    // Raw message data input changes to Model
    connect(m_rawView->messageDataEditor(), &QLineEdit::textChanged, this,
            [model](const QString& text) {
                std::vector<uint8_t> data;

                if (const QString messageDataText = text.trimmed(); !messageDataText.isEmpty())
                {
                    const QStringList byteStrings =
                        messageDataText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

                    for (int i = 0;
                         i < byteStrings.size() && i < static_cast<int>(Constants::MAX_CAN_DLC);
                         ++i)
                    {
                        bool ok = false;
                        const uint8_t byte = byteStrings[i].toUInt(&ok, 16);
                        if (ok)
                        {
                            data.push_back(byte);
                        }
                    }
                }
                model->setRawData(data);
            });

    // Raw Send Button to Model Transmit
    connect(m_rawView->sendButton(), &QPushButton::clicked, this,
            [model]() { model->transmitCurrent(); });

    // DBC Send Button to Model Transmit
    connect(m_dbcView->sendButton(), &QPushButton::clicked, this,
            [model]() { model->transmitCurrent(); });

    // Forward model's send requests to view signals
    connect(model, &SendingModel::requestSendRaw, this,
            [this](const std::string&, const Core::RawCanMessage& message) {
                emit sendRawRequested(message);
            });

    connect(model, &SendingModel::requestSendDbc, this,
            [this](const std::string&, const Core::DbcCanMessage& message) {
                emit sendDbcRequested(message);
            });

    // Initial button state
    updateSendButtonStates();
}

void SendingView::updateSendButtonStates() const
{
    if (auto* rawSendBtn = m_rawView->sendButton())
    {
        rawSendBtn->setEnabled(m_rawInterfaceSelected);
    }

    if (auto* dbcSendBtn = m_dbcView->sendButton())
    {
        bool anySignalSelected = false;
        if (m_model && m_model->currentDbcConfig())
        {
            for (const auto& msg : m_model->currentDbcConfig()->messageDefinitions)
            {
                for (const auto& sig : msg.signalDescriptions)
                {
                    if (m_model->isSignalSelected(msg.messageId, sig.signalName))
                    {
                        anySignalSelected = true;
                        break;
                    }
                }
                if (anySignalSelected)
                {
                    break;
                }
            }
        }

        dbcSendBtn->setEnabled(m_dbcInterfaceSelected && anySignalSelected);
    }
}

void SendingView::setAvailableDevices(const std::vector<std::string>& devices) const
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

}  // namespace Sending
