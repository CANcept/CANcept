#include "sending_view.hpp"

#include <QHBoxLayout>
#include <QList>
#include <QRegularExpression>
#include <QVBoxLayout>

#include "sending/constants.hpp"

namespace Sending {

SendingView::SendingView(QWidget* parent)
    : QWidget(parent),
      m_sidebar(nullptr),
      m_contentStack(nullptr),
      m_rawView(nullptr),
      m_dbcView(nullptr)
{
    setupUi();
}

void SendingView::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_sidebar = new Core::Sidebar(this);
    m_sidebar->addTab(QIcon(Constants::RAW_SENDING_ICON_PATH), Constants::RAW_MODE_BUTTON_TEXT);
    m_sidebar->addTab(QIcon(Constants::DBC_SENDING_ICON_PATH), Constants::DBC_MODE_BUTTON_TEXT);
    mainLayout->addWidget(m_sidebar);

    m_contentStack = new QStackedWidget(this);

    // Create sub-views
    m_rawView = new RawSendingSubView(m_contentStack);
    m_dbcView = new DbcSendingSubView(m_contentStack);

    m_contentStack->addWidget(m_rawView);  // Index 0
    m_contentStack->addWidget(m_dbcView);  // Index 1

    mainLayout->addWidget(m_contentStack, 1);

    connect(m_sidebar, &Core::Sidebar::tabSelected, this, &SendingView::displayMode);
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

    // Initial button state
    updateSendButtonStates();
}

void SendingView::updateSendButtonStates() const
{
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

        dbcSendBtn->setEnabled(anySignalSelected);
    }
}

}  // namespace Sending
