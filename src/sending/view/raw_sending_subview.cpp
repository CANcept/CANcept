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

#include "raw_sending_subview.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

#include "components/hex_id_line_edit.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "sending/constants.hpp"
#include "sending/view/formatter/hex_data_formatter.hpp"

namespace Sending {

RawSendingSubView::RawSendingSubView(QWidget* parent)
    : QWidget(parent),
      m_scrollArea(nullptr),
      m_frameCard(nullptr),
      m_canIdEditor(nullptr),
      m_messageDataEditor(nullptr),
      m_messageDataFormatter(nullptr),
      m_canIdLabel(nullptr),
      m_messageDataLabel(nullptr),
      m_repeatedSendingCard(nullptr),
      m_manipulation(nullptr),
      m_sendButton(nullptr)
{
    setupUi();
}

void RawSendingSubView::setupUi()
{
    const auto& spacing = THEME.spacing();

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* scrollContent = new QWidget(this);
    m_scrollArea->setWidget(scrollContent);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);

    auto* contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                      spacing.spacingLg);
    contentLayout->setSpacing(spacing.spacingLg);

    // CAN Frame Card
    m_frameCard = new Core::CardWidget(Constants::CAN_FRAME_TITLE, Constants::CAN_FRAME_DESCRIPTION,
                                       Constants::CAN_FRAME_ICON_PATH, this);
    auto* frameCardLayout = m_frameCard->contentLayout();

    // CAN ID Input
    m_canIdLabel = new QLabel(Constants::CAN_ID_LABEL, m_frameCard);
    m_canIdEditor = new HexIdLineEdit(m_frameCard);
    frameCardLayout->addWidget(m_canIdLabel);
    frameCardLayout->addWidget(m_canIdEditor);

    frameCardLayout->addSpacing(spacing.spacingMd);

    // Message Data Input
    m_messageDataLabel = new QLabel(Constants::MESSAGE_DATA_LABEL, m_frameCard);
    m_messageDataEditor = new Core::StyledLineEdit(m_frameCard);
    m_messageDataEditor->setPlaceholderText(Constants::MESSAGE_DATA_PLACEHOLDER);
    frameCardLayout->addWidget(m_messageDataLabel);
    frameCardLayout->addWidget(m_messageDataEditor);

    // Setup input validation and formatting
    setupCanIdInput();
    setupMessageDataInput();
    contentLayout->addWidget(m_frameCard);

    // Repeated Sending Card
    m_repeatedSendingCard = new RepeatedSendingCard(this);
    contentLayout->addWidget(m_repeatedSendingCard);

    // Manipulation (raw-only: no DBC context is available on this tab)
    m_manipulation = new Manipulation::ManipulationView(this);
    m_manipulation->setMode(Manipulation::ManipulationModel::Mode::Raw);
    m_manipulation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    const int manipulationMaxHeight =
        static_cast<int>(this->screen()->availableGeometry().height() * 0.3);
    m_manipulation->setMaximumHeight(manipulationMaxHeight);
    contentLayout->addWidget(m_manipulation);

    contentLayout->addStretch();

    // Send Message button in the bottom right corner
    auto* buttonContainer = new QWidget(this);
    buttonContainer->setFixedHeight(Constants::BUTTON_CONTAINER_HEIGHT);

    auto* buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                     spacing.spacingLg);
    buttonLayout->addStretch();

    m_sendButton = new SendMessageButton(buttonContainer);
    buttonLayout->addWidget(m_sendButton);
    mainLayout->addWidget(buttonContainer);

    applyStyle();
}

void RawSendingSubView::applyStyle()
{
    const auto& colors = THEME.colors();

    m_scrollArea->setStyleSheet(QString("background-color: %1;").arg(colors.surfaceMain.name()));
    m_canIdLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    m_messageDataLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
}

bool RawSendingSubView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void RawSendingSubView::setupCanIdInput() const
{
    if (!m_canIdEditor)
    {
        return;
    }
    m_canIdEditor->setMaxHexValue(Constants::MAX_CAN_ID);
}

void RawSendingSubView::setupMessageDataInput()
{
    if (!m_messageDataEditor)
    {
        return;
    }
    m_messageDataFormatter = new HexDataFormatter(m_messageDataEditor, Constants::MAX_CAN_DLC);
}

}  // namespace Sending
