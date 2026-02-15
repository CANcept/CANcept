#include "repeated_sending_card.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "sending/constants.hpp"
#include "sending/view/validator/hex_validator.hpp"

namespace Sending {

RepeatedSendingCard::RepeatedSendingCard(QWidget* parent)
    : QWidget(parent),
      m_card(nullptr),
      m_titleLabel(nullptr),
      m_subtitleLabel(nullptr),
      m_toggleSwitch(nullptr),
      m_frequencyEditor(nullptr),
      m_frequencyLabel(nullptr)
{
    setupUi();
}

void RepeatedSendingCard::setupUi()
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

    m_titleLabel = new QLabel(Constants::REPEATED_SENDING_TITLE, m_card);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(spacing.fontSizeSm);
    titleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightNormal));
    m_titleLabel->setFont(titleFont);
    textLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(Constants::REPEATED_SENDING_SUBTITLE, m_card);
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

    // Frequency input
    m_frequencyLabel = new QLabel(Constants::REPEATED_SENDING_TRANSMISSION_INPUT_TITLE, m_card);
    QFont labelFont = m_frequencyLabel->font();
    labelFont.setPointSize(spacing.fontSizeXs);
    m_frequencyLabel->setFont(labelFont);
    m_frequencyLabel->setVisible(false);
    cardLayout->addWidget(m_frequencyLabel);

    m_frequencyEditor = new Core::StyledLineEdit(m_card);
    m_frequencyEditor->setPlaceholderText(QString::number(Constants::DEFAULT_CYCLE_INTERVAL_MS));
    const auto* validator = new QIntValidator(0, Constants::REPEATED_SENDING_MAX_FREQUENCY, this);
    m_frequencyEditor->setValidator(validator);
    m_frequencyEditor->setVisible(false);
    cardLayout->addWidget(m_frequencyEditor);

    mainLayout->addWidget(m_card);

    // Connect toggle signal
    connect(m_toggleSwitch, &Core::StyledSwitch::toggled, this,
            &RepeatedSendingCard::onToggleChanged);

    applyStyle();
}

void RepeatedSendingCard::applyStyle() const
{
    const auto& colors = THEME.colors();

    // Apply text colors
    if (m_titleLabel)
    {
        m_titleLabel->setStyleSheet(QString("color: %1;").arg(colors.textPrimary.name()));
    }

    if (m_subtitleLabel)
    {
        m_subtitleLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    }

    if (m_frequencyLabel)
    {
        m_frequencyLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    }
}

bool RepeatedSendingCard::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void RepeatedSendingCard::onToggleChanged(const bool checked)
{
    m_frequencyLabel->setVisible(checked);
    m_frequencyEditor->setVisible(checked);
    emit toggled(checked);
}

auto RepeatedSendingCard::isRepeatedSendingEnabled() const -> bool
{
    return m_toggleSwitch->isChecked();
}

}  // namespace Sending
