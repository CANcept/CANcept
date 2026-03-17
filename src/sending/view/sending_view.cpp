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

#include "sending_view.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QRegularExpression>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "sending/constants.hpp"
#include "sending/view/components/repeated_sending_card.hpp"
#include "sending/view/components/send_message_button.hpp"

namespace Sending {

SendingView::SendingView(QWidget* parent)
    : QWidget(parent),
      m_sidebar(nullptr),
      m_contentStack(nullptr),
      m_rawView(nullptr),
      m_dbcView(nullptr),
      m_deviceNotConfiguredOverlay(nullptr),
      m_settingsIconLabel(nullptr)
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

    m_deviceNotConfiguredOverlay = new QWidget(this);
    m_deviceNotConfiguredOverlay->setObjectName("deviceNotConfiguredOverlay");

    auto* overlayLayout = new QVBoxLayout(m_deviceNotConfiguredOverlay);

    overlayLayout->addStretch(2);

    auto* messageContainer = new QWidget(m_deviceNotConfiguredOverlay);
    auto* messageLayout = new QHBoxLayout(messageContainer);
    messageLayout->setAlignment(Qt::AlignCenter);

    auto* messageLabel = new QLabel(Constants::NO_DEVICE_CONFIGURED_TEXT, messageContainer);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLayout->addWidget(messageLabel);

    const auto& colors = THEME.colors();
    m_settingsIconLabel = new Core::TintedIconLabel(Constants::SETTINGS_ICON_PATH, 24,
                                                    colors.textPrimary, messageContainer);
    messageLayout->addWidget(m_settingsIconLabel);

    overlayLayout->addWidget(messageContainer, 0, Qt::AlignCenter);

    overlayLayout->addStretch(3);

    m_deviceNotConfiguredOverlay->hide();
    m_deviceNotConfiguredOverlay->raise();

    applyStyle();
}

void SendingView::displayMode(const int index)
{
    m_contentStack->setCurrentIndex(index);
    emit modeChanged(index == 1);
}

void SendingView::applyStyle() const
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->setStyleSheet(
            QString("QWidget#deviceNotConfiguredOverlay { "
                    "background-color: rgba(%1, %2, %3, 140); "
                    "}"
                    "QLabel { "
                    "color: %4; "
                    "font-size: %5px; "
                    "}")
                .arg(colors.surfaceMain.red())
                .arg(colors.surfaceMain.green())
                .arg(colors.surfaceMain.blue())
                .arg(colors.textPrimary.name())
                .arg(spacing.fontSizeLg));
    }

    if (m_settingsIconLabel)
    {
        m_settingsIconLabel->setColor(colors.textPrimary);
    }
}

bool SendingView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
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

    // Raw View: Repeated Sending Toggle
    connect(m_rawView->repeatedSendingCard(), &RepeatedSendingCard::toggled, this,
            [model](const bool enabled) {
                model->setData(QModelIndex(), enabled, SendingModel::Role_IsCyclicEnabled);
            });

    // Raw View: Repeated Sending Interval
    connect(m_rawView->repeatedSendingCard()->frequencyEditor(), &QLineEdit::textChanged, this,
            [model](const QString& text) {
                bool ok = false;
                if (const int interval = text.toInt(&ok); ok && interval > 0)
                {
                    model->setData(QModelIndex(), interval, SendingModel::Role_CycleIntervalMs);
                }
            });

    // Raw Send Button: Start/Stop repeated sending or send once
    connect(m_rawView->sendButton(), &QPushButton::clicked, this, [this]() {
        if (m_rawView->repeatedSendingCard()->isRepeatedSendingEnabled())
        {
            // Toggle repeated sending state
            if (m_model->isCurrentlySending())
            {
                emit stopRepeatedSendingRequested();
            } else
            {
                // REtrieve interval from frequency editor
                bool ok = false;
                const int interval =
                    m_rawView->repeatedSendingCard()->frequencyEditor()->text().toInt(&ok);
                const int safeInterval =
                    ok && interval > 0 ? interval : Constants::DEFAULT_CYCLE_INTERVAL_MS;
                emit startRepeatedSendingRequested(safeInterval);
            }
        } else
        {
            emit sendOnceRequested();
        }
    });

    // DBC View: Repeated Sending Toggle
    connect(m_dbcView->repeatedSendingCard(), &RepeatedSendingCard::toggled, this,
            [model](const bool enabled) {
                model->setData(QModelIndex(), enabled, SendingModel::Role_IsCyclicEnabled);
            });

    // DBC View: Repeated Sending Interval
    connect(m_dbcView->repeatedSendingCard()->frequencyEditor(), &QLineEdit::textChanged, this,
            [model](const QString& text) {
                bool ok = false;
                if (const int interval = text.toInt(&ok); ok && interval > 0)
                {
                    model->setData(QModelIndex(), interval, SendingModel::Role_CycleIntervalMs);
                }
            });

    // DBC Send Button: Start/Stop repeated sending or send once
    connect(m_dbcView->sendButton(), &QPushButton::clicked, this, [this]() {
        if (m_dbcView->repeatedSendingCard()->isRepeatedSendingEnabled())
        {
            // Toggle repeated sending state
            if (m_model->isCurrentlySending())
            {
                emit stopRepeatedSendingRequested();
            } else
            {
                // Get interval from the frequency editor
                bool ok = false;
                const int interval =
                    m_dbcView->repeatedSendingCard()->frequencyEditor()->text().toInt(&ok);
                const int safeInterval =
                    ok && interval > 0 ? interval : Constants::DEFAULT_CYCLE_INTERVAL_MS;
                emit startRepeatedSendingRequested(safeInterval);
            }
        } else
        {
            // Send once
            emit sendOnceRequested();
        }
    });

    // Update button appearance based on sending state
    connect(model, &QAbstractItemModel::dataChanged, this,
            [this, model](const QModelIndex&, const QModelIndex&, const QList<int>& roles) {
                if (roles.contains(SendingModel::Role_IsCurrentlySending))
                {
                    const bool isSending = model->isCurrentlySending();
                    if (auto* rawBtn = dynamic_cast<SendMessageButton*>(m_rawView->sendButton()))
                    {
                        rawBtn->setSendingState(isSending);
                    }
                    if (auto* dbcBtn = dynamic_cast<SendMessageButton*>(m_dbcView->sendButton()))
                    {
                        dbcBtn->setSendingState(isSending);
                    }
                }
            });

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

void SendingView::showDeviceNotConfiguredOverlay() const
{
    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->setGeometry(0, 0, width(), height());
        m_deviceNotConfiguredOverlay->show();
        m_deviceNotConfiguredOverlay->raise();
    }
}

void SendingView::hideDeviceNotConfiguredOverlay() const
{
    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->hide();
    }
}

void SendingView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->setGeometry(0, 0, width(), height());
    }
}

}  // namespace Sending
