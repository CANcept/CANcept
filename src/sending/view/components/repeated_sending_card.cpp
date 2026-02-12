#include "repeated_sending_card.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "sending/constants.hpp"

namespace Sending {

RepeatedSendingCard::RepeatedSendingCard(QWidget* parent)
    : QWidget(parent),
      m_card(nullptr),
      m_iconLabel(nullptr),
      m_titleLabel(nullptr),
      m_subtitleLabel(nullptr),
      m_toggleCheckbox(nullptr),
      m_frequencyEditor(nullptr),
      m_frequencyLabel(nullptr)
{
    setupUi();
}

void RepeatedSendingCard::setupUi()
{
    const auto& spacing = THEME.spacing();

    // Main layout for the widget
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create card without title/subtitle (we'll add custom header)
    m_card = new Core::CardWidget(QString(), QString(), QString(), this);
    auto* cardLayout = m_card->contentLayout();

    // Custom header with icon, title/subtitle, and toggle
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(spacing.spacingSm);

    // Icon
    m_iconLabel = new QLabel(m_card);
    headerLayout->addWidget(m_iconLabel, 0, Qt::AlignVCenter);

    // Title and subtitle in vertical layout
    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(spacing.spacingXs);
    textLayout->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new QLabel("Repeated Sending", m_card);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(spacing.fontSizeSm);
    titleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightNormal));
    m_titleLabel->setFont(titleFont);
    textLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel("Enable cyclic message transmission", m_card);
    QFont subtitleFont = m_subtitleLabel->font();
    subtitleFont.setPointSize(spacing.fontSizeXs);
    subtitleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightMedium));
    m_subtitleLabel->setFont(subtitleFont);
    textLayout->addWidget(m_subtitleLabel);

    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();

    // Toggle checkbox on the right
    m_toggleCheckbox = new Core::StyledCheckBox(m_card);
    headerLayout->addWidget(m_toggleCheckbox, 0, Qt::AlignVCenter);

    // Add header to card as first element
    cardLayout->insertLayout(0, headerLayout);

    // Frequency input section (initially hidden)
    m_frequencyLabel = new QLabel("Transmission Interval (ms)", m_card);
    QFont labelFont = m_frequencyLabel->font();
    labelFont.setPointSize(spacing.fontSizeXs);
    m_frequencyLabel->setFont(labelFont);
    m_frequencyLabel->setVisible(false);
    cardLayout->addWidget(m_frequencyLabel);

    m_frequencyEditor = new Core::StyledLineEdit(m_card);
    m_frequencyEditor->setPlaceholderText(QString::number(Constants::DEFAULT_CYCLE_INTERVAL_MS));
    m_frequencyEditor->setVisible(false);
    cardLayout->addWidget(m_frequencyEditor);

    mainLayout->addWidget(m_card);

    // Connect toggle signal
    connect(m_toggleCheckbox, &Core::StyledCheckBox::toggled, this,
            &RepeatedSendingCard::onToggleChanged);

    applyStyle();
}

void RepeatedSendingCard::applyStyle()
{
    const auto& colors = THEME.colors();

    // Apply icon with theme color
    const QString iconPath = Constants::SEND_BUTTON_ICON_PATH;
    if (m_iconLabel && !iconPath.isEmpty())
    {
        constexpr int iconHeight = 20;
        const qreal dpr = devicePixelRatioF();

        const QIcon icon(iconPath);
        const QSize actualSize = icon.actualSize(QSize(iconHeight * 2, iconHeight));
        QPixmap pixmap = icon.pixmap(actualSize, dpr);

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), colors.surfaceForeground);
        painter.end();

        m_iconLabel->setPixmap(pixmap);
    }

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

void RepeatedSendingCard::onToggleChanged(bool checked)
{
    // Show/hide frequency input based on toggle state
    m_frequencyLabel->setVisible(checked);
    m_frequencyEditor->setVisible(checked);
}

auto RepeatedSendingCard::isRepeatedSendingEnabled() const -> bool
{
    return m_toggleCheckbox->isChecked();
}

}  // namespace Sending
