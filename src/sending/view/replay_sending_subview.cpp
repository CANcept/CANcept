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

#include "replay_sending_subview.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <algorithm>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/common/styled_combo_box.hpp"
#include "sending/constants.hpp"
#include "sending/view/components/replay_control_button.hpp"
#include "sending/view/components/replay_progress_bar.hpp"

namespace Sending {

namespace {
auto sessionTypeToText(const Core::ReplaySessionType type) -> QString
{
    return type == Core::ReplaySessionType::RAW ? "RAW" : "DBC";
}

auto stateToText(const ReplaySendingSubView::LoadState state) -> QString
{
    switch (state)
    {
        case ReplaySendingSubView::LoadState::SessionReady:
            return Constants::REPLAY_STATUS_SESSION_READY;
        case ReplaySendingSubView::LoadState::Loading:
            return Constants::REPLAY_STATUS_LOADING;
        case ReplaySendingSubView::LoadState::Loaded:
            return Constants::REPLAY_STATUS_LOADED;
        case ReplaySendingSubView::LoadState::NoSessions:
            return Constants::REPLAY_STATUS_NO_SESSIONS;
        case ReplaySendingSubView::LoadState::Error:
            return Constants::REPLAY_STATUS_ERROR;
    }
    return Constants::REPLAY_STATUS_NO_SESSIONS;
}
}  // namespace

ReplaySendingSubView::ReplaySendingSubView(QWidget* parent)
    : QWidget(parent),
      m_scrollArea(nullptr),
      m_sessionCard(nullptr),
      m_sessionCombo(nullptr),
      m_sessionDetailsLabel(nullptr),
      m_loadFramesButton(nullptr),
      m_loadStateCard(nullptr),
      m_loadStatusLabel(nullptr),
      m_warningLabel(nullptr),
      m_playbackCard(nullptr),
      m_startButton(nullptr),
      m_pauseButton(nullptr),
      m_resumeButton(nullptr),
      m_stopButton(nullptr),
      m_speedCombo(nullptr),
      m_progressCard(nullptr),
      m_progressBar(nullptr),
      m_progressTextLabel(nullptr)
{
    setupUi();
}

void ReplaySendingSubView::setupUi()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* scrollContent = new QWidget(this);
    m_scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(m_scrollArea);

    auto* contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                      spacing.spacingLg);
    contentLayout->setSpacing(spacing.spacingLg);

    // Section 1: Replay session selection card
    m_sessionCard = new Core::CardWidget(Constants::REPLAY_SESSIONS_TITLE,
                                         Constants::REPLAY_SESSIONS_SUBTITLE, QString(), this);

    auto* sessionCardLayout = m_sessionCard->contentLayout();

    m_sessionCombo = new Core::StyledComboBox(m_sessionCard);
    m_sessionCombo->setPlaceholderText(Constants::SESSIONS_COMBO_PLACEHOLDER_EMPTY);

    m_sessionDetailsLabel = new QLabel(Constants::NO_SESSION_DETAILS_TEXT, m_sessionCard);
    m_sessionDetailsLabel->setWordWrap(true);

    m_loadFramesButton = new ReplayControlButton(
        Constants::LOAD_FRAMES_BUTTON_TEXT, ReplayControlButton::Variant::Primary, m_sessionCard);
    m_loadFramesButton->setEnabled(false);

    sessionCardLayout->addWidget(m_sessionCombo);
    sessionCardLayout->addWidget(m_sessionDetailsLabel);
    sessionCardLayout->addWidget(m_loadFramesButton, 0, Qt::AlignRight);

    contentLayout->addWidget(m_sessionCard);

    // Section 2: Frame loading state
    m_loadStateCard = new Core::CardWidget(Constants::REPLAY_LOADING_TITLE,
                                           Constants::REPLAY_LOADING_SUBTITLE, QString(), this);
    auto* loadStateLayout = m_loadStateCard->contentLayout();

    m_loadStatusLabel = new QLabel(stateToText(LoadState::NoSessions), m_loadStateCard);
    m_loadStatusLabel->setWordWrap(true);

    m_warningLabel = new QLabel(QString(), m_loadStateCard);
    m_warningLabel->setWordWrap(true);
    m_warningLabel->hide();

    loadStateLayout->addWidget(m_loadStatusLabel);
    loadStateLayout->addWidget(m_warningLabel);
    contentLayout->addWidget(m_loadStateCard);

    // Section 3: Playback controls
    m_playbackCard = new Core::CardWidget(Constants::REPLAY_PLAYBACK_TITLE,
                                          Constants::REPLAY_PLAYBACK_SUBTITLE, QString(), this);
    auto* playbackLayout = m_playbackCard->contentLayout();

    auto* buttonRow = new QWidget(m_playbackCard);
    auto* buttonRowLayout = new QHBoxLayout(buttonRow);
    buttonRowLayout->setContentsMargins(0, 0, 0, 0);

    m_startButton = new ReplayControlButton(Constants::REPLAY_START_BUTTON_TEXT,
                                            ReplayControlButton::Variant::Primary, buttonRow);
    m_pauseButton = new ReplayControlButton(Constants::REPLAY_PAUSE_BUTTON_TEXT,
                                            ReplayControlButton::Variant::Secondary, buttonRow);
    m_pauseButton->setEnabled(false);
    m_resumeButton = new ReplayControlButton(Constants::REPLAY_RESUME_BUTTON_TEXT,
                                             ReplayControlButton::Variant::Secondary, buttonRow);
    m_resumeButton->setEnabled(false);
    m_stopButton = new ReplayControlButton(Constants::REPLAY_STOP_BUTTON_TEXT,
                                           ReplayControlButton::Variant::Danger, buttonRow);
    m_stopButton->setEnabled(false);

    buttonRowLayout->addWidget(m_startButton);
    buttonRowLayout->addWidget(m_pauseButton);
    buttonRowLayout->addWidget(m_resumeButton);
    buttonRowLayout->addWidget(m_stopButton);

    playbackLayout->addWidget(buttonRow);

    // Speed selector
    QLabel* speedLabel;
    speedLabel = new QLabel(Constants::REPLAY_SPEED_LABEL_TEXT, m_playbackCard);
    m_speedCombo = new Core::StyledComboBox(m_playbackCard);
    m_speedCombo->addItem(Constants::REPLAY_SPEED_OPTION_025X);
    m_speedCombo->addItem(Constants::REPLAY_SPEED_OPTION_05X);
    m_speedCombo->addItem(Constants::REPLAY_SPEED_OPTION_1X);
    m_speedCombo->addItem(Constants::REPLAY_SPEED_OPTION_2X);
    m_speedCombo->setCurrentIndex(2);  // Default to 1x

    auto* speedRow = new QWidget(m_playbackCard);
    auto* speedRowLayout = new QHBoxLayout(speedRow);
    speedRowLayout->setContentsMargins(0, 0, 0, 0);
    speedRowLayout->addWidget(speedLabel);
    speedRowLayout->addWidget(m_speedCombo);
    speedRowLayout->addStretch();

    playbackLayout->addWidget(speedRow);
    contentLayout->addWidget(m_playbackCard);

    // Section 4: Progress & status
    m_progressCard = new Core::CardWidget(Constants::REPLAY_PROGRESS_TITLE,
                                          Constants::REPLAY_PROGRESS_SUBTITLE, QString(), this);
    auto* progressLayout = m_progressCard->contentLayout();

    m_progressBar = new ReplayProgressBar(m_progressCard);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);

    m_progressTextLabel =
        new QLabel(Constants::REPLAY_PROGRESS_TEXT_TEMPLATE.arg(0).arg(0), m_progressCard);
    m_progressTextLabel->setWordWrap(true);

    progressLayout->addWidget(m_progressBar);
    progressLayout->addWidget(m_progressTextLabel);

    contentLayout->addWidget(m_progressCard);

    contentLayout->addStretch();

    connect(m_sessionCombo, &QComboBox::currentIndexChanged, this, [this](int /*index*/) {
        const int current = m_sessionCombo->currentIndex();
        const bool hasSelection = current >= 0 && current < m_sessions.size();
        if (m_loadFramesButton)
        {
            m_loadFramesButton->setEnabled(hasSelection);
        }
        updateSessionDetailsLabel();
    });

    connect(m_loadFramesButton, &QPushButton::clicked, this, [this]() {
        const int index = m_sessionCombo->currentIndex();
        if (index < 0 || index >= m_sessions.size())
        {
            return;
        }
        setLoadState(LoadState::Loading);
        clearWarningText();
        setPlaybackState(PlaybackState::Disabled);
        setProgress(0, 0);
        emit loadFramesRequested(m_sessions.at(index).sessionId);
    });

    connect(m_startButton, &QPushButton::clicked, this,
            [this]() { emit startReplayRequested(selectedSpeedFactor()); });
    connect(m_pauseButton, &QPushButton::clicked, this, [this]() { emit pauseReplayRequested(); });
    connect(m_resumeButton, &QPushButton::clicked, this,
            [this]() { emit resumeReplayRequested(); });
    connect(m_stopButton, &QPushButton::clicked, this, [this]() { emit stopReplayRequested(); });

    setPlaybackState(PlaybackState::Disabled);
    setProgress(0, 0);
    applyStyle();
}

void ReplaySendingSubView::setSessions(const QList<Core::ReplaySessionInfo>& sessions)
{
    m_sessions = sessions;

    m_sessionCombo->blockSignals(true);
    m_sessionCombo->clear();

    for (const auto& session : m_sessions)
    {
        m_sessionCombo->addItem(session.displayName);
    }

    // No automatic selection: the user has to pick a session explicitly.
    m_sessionCombo->setCurrentIndex(-1);
    m_sessionCombo->blockSignals(false);

    const bool hasSessions = !m_sessions.isEmpty();
    m_loadFramesButton->setEnabled(false);

    if (hasSessions)
    {
        m_sessionCombo->setPlaceholderText(Constants::SESSIONS_COMBO_PLACEHOLDER);
        setLoadState(LoadState::SessionReady);
        m_sessionDetailsLabel->setText(Constants::NO_SESSION_DETAILS_TEXT);
    } else
    {
        m_sessionCombo->setPlaceholderText(Constants::SESSIONS_COMBO_PLACEHOLDER_EMPTY);
        setLoadState(LoadState::NoSessions);
        clearWarningText();
        m_sessionDetailsLabel->setText(Constants::NO_SESSION_DETAILS_TEXT);
    }

    setPlaybackState(PlaybackState::Disabled);
    setProgress(0, 0);
    updateSessionDetailsLabel();
}

void ReplaySendingSubView::setLoadState(const LoadState state)
{
    if (m_loadStatusLabel)
    {
        m_loadStatusLabel->setText(stateToText(state));
    }
}

void ReplaySendingSubView::setWarningText(const QString& text)
{
    if (!m_warningLabel)
    {
        return;
    }

    if (text.trimmed().isEmpty())
    {
        m_warningLabel->clear();
        m_warningLabel->hide();
        return;
    }

    m_warningLabel->setText(text);
    m_warningLabel->show();
}

void ReplaySendingSubView::clearWarningText()
{
    setWarningText(QString());
}

void ReplaySendingSubView::updateSessionDetailsLabel()
{
    const int index = m_sessionCombo->currentIndex();
    if (index < 0 || index >= m_sessions.size())
    {
        m_sessionDetailsLabel->setText(m_sessions.isEmpty()
                                           ? Constants::SESSIONS_COMBO_PLACEHOLDER_EMPTY
                                           : Constants::SESSIONS_COMBO_PLACEHOLDER);
        return;
    }

    const auto& session = m_sessions.at(index);
    m_sessionDetailsLabel->setText(QString("Type: %1 | Duration: %2 ms | Frames: %3")
                                       .arg(sessionTypeToText(session.type))
                                       .arg(session.duration.count())
                                       .arg(session.frameCount));
}

void ReplaySendingSubView::applyStyle() const
{
    const auto& colors = THEME.colors();

    m_scrollArea->setStyleSheet(QString("background-color: %1;").arg(colors.surfaceMain.name()));

    if (m_sessionDetailsLabel)
    {
        m_sessionDetailsLabel->setStyleSheet(
            QString("color: %1;").arg(colors.textSecondary.name()));
    }
    if (m_loadStatusLabel)
    {
        m_loadStatusLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    }
    if (m_warningLabel)
    {
        m_warningLabel->setStyleSheet(QString("color: %1;").arg(colors.statusError.name()));
    }
    if (m_progressTextLabel)
    {
        m_progressTextLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    }
}

auto ReplaySendingSubView::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QWidget::event(event);
}

void ReplaySendingSubView::setPlaybackState(const PlaybackState state)
{
    m_playbackState = state;

    const bool hasSessionList = !m_sessions.isEmpty();
    const bool startEnabled = hasSessionList && (state == PlaybackState::Ready);
    const bool pauseEnabled = (state == PlaybackState::Running);
    const bool resumeEnabled = (state == PlaybackState::Paused);
    const bool stopEnabled = (state == PlaybackState::Running || state == PlaybackState::Paused);

    if (m_startButton) m_startButton->setEnabled(startEnabled);
    if (m_pauseButton) m_pauseButton->setEnabled(pauseEnabled);
    if (m_resumeButton) m_resumeButton->setEnabled(resumeEnabled);
    if (m_stopButton) m_stopButton->setEnabled(stopEnabled);

    // Disable speed combo while replay is running
    const bool isRunning = (state == PlaybackState::Running || state == PlaybackState::Paused);
    if (m_speedCombo) m_speedCombo->setEnabled(!isRunning);
}

void ReplaySendingSubView::setProgress(const int currentFrame, const int totalFrames)
{
    const int safeTotal = std::max(0, totalFrames);
    const int safeCurrent = std::clamp(currentFrame, 0, safeTotal);
    const int progressPercent = (safeTotal > 0) ? ((safeCurrent * 100) / safeTotal) : 0;

    if (m_progressBar)
    {
        m_progressBar->setValue(progressPercent);
    }

    if (m_progressTextLabel)
    {
        m_progressTextLabel->setText(
            Constants::REPLAY_PROGRESS_TEXT_TEMPLATE.arg(safeCurrent).arg(safeTotal));
    }
}

auto ReplaySendingSubView::selectedSpeedFactor() const -> double
{
    if (!m_speedCombo)
    {
        return 1.0;
    }

    switch (m_speedCombo->currentIndex())
    {
        case 0:
            return 0.25;
        case 1:
            return 0.5;
        case 2:
            return 1.0;
        case 3:
            return 2.0;
        default:
            return 1.0;
    }
}

}  // namespace Sending
