#include "load_page.hpp"

#include <QDragEnterEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/theme/theme_manager.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/styles.hpp"
#include "dbc_file/util/util.hpp"

namespace DbcFile {
LoadPage::LoadPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
LoadPage::~LoadPage() = default;

namespace {

/**
 * @brief Updates the visual style of a widget based on a drag state.
 *
 * @param widget The widget whose style will be updated.
 * @param state The new drag state ("valid", "invalid", or "").
 *
 * @details Calls unpolish() and polish() to force Qt to re-evaluate dynamic
 * stylesheet selectors like [dragState="valid"].
 */
void updateDragStyle(QWidget* widget, const QString& state)
{
    widget->setProperty(Constants::LoadPage::Drag::Property, state);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

/**
 * @brief Creates an instruction label for the upload zone.
 * @param parent Parent widget.
 * @return QLabel containing the instruction text.
 */
auto createUploadInstruction(QWidget* parent) -> QLabel*
{
    auto* textLabel = new QLabel(parent);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setText(Constants::LoadPage::CardInstruction);
    return textLabel;
}
}  // namespace

void LoadPage::showStatusMessage(const QString& message, const bool isError) const
{
    m_statusLabel->setText(message);
    m_statusLabel->show();
    m_statusLabel->setStyleSheet(Style::LoadPage::statusLabel(isError));
    // updateDragStyle(m_uploadBoxFrame, DragStateNone);
}
void LoadPage::resetStatus() const
{
    if (m_statusLabel->isVisible())
    {
        m_statusLabel->hide();
        m_statusLabel->clear();
    }
    updateDragStyle(m_uploadBoxFrame, Constants::LoadPage::Drag::None);
}

void LoadPage::dragEnterEvent(QDragEnterEvent* event)
{
    resetStatus();

    QList<QString> filePaths;
    for (const QUrl& url : event->mimeData()->urls())
    {
        filePaths.append(url.toLocalFile());
    }

    if (Util::canAcceptDrop(filePaths))
    {
        updateDragStyle(m_uploadBoxFrame, Constants::LoadPage::Drag::Valid);
    } else
    {
        updateDragStyle(m_uploadBoxFrame, Constants::LoadPage::Drag::Invalid);
    }

    event->acceptProposedAction();
}

void LoadPage::dropEvent(QDropEvent* event)
{
    // Return to default style of upload zone and hide status label
    resetStatus();

    const auto urls = event->mimeData()->urls();
    if (urls.isEmpty())
    {
        return;
    }

    // Too many files warning
    if (urls.size() > 1)
    {
        showStatusMessage(Constants::LoadPage::Errors::TooManyFiles, true);
        return;
    }

    const QString filePath = urls.first().toLocalFile();
    // Wrong ending warning
    if (!Util::isValidFile(filePath))
    {
        showStatusMessage(Constants::LoadPage::Errors::InvalidFileBody, true);
        return;
    }

    // Success
    showStatusMessage(Constants::LoadPage::StatusParsing, false);
    emit fileSelected(filePath);
    event->acceptProposedAction();
}
void LoadPage::dragLeaveEvent(QDragLeaveEvent* event)
{
    // Change upload box style to default when cursor leaves upload box
    updateDragStyle(m_uploadBoxFrame, Constants::LoadPage::Drag::None);
    event->accept();
}
auto LoadPage::eventFilter(QObject* watched, QEvent* event) -> bool
{
    if (watched == m_uploadBoxFrame)
    {
        switch (event->type())
        {
            case QEvent::DragEnter:
                dragEnterEvent(static_cast<QDragEnterEvent*>(event));
                return true;

            case QEvent::DragLeave:
                dragLeaveEvent(static_cast<QDragLeaveEvent*>(event));
                return true;

            case QEvent::Drop:
                dropEvent(static_cast<QDropEvent*>(event));
                return true;

            case QEvent::MouseButtonRelease:
                onBrowseButtonClicked();
                return true;

            default:
                break;
        }
    }
    return QWidget::eventFilter(watched, event);
}
void LoadPage::onBrowseButtonClicked()
{
    resetStatus();
    const QString fileName =
        QFileDialog::getOpenFileName(this, tr(Constants::LoadPage::FileDialogTitle), QString(),
                                     tr(Constants::LoadPage::FileDialogFilter));

    if (fileName.isEmpty()) return;  // file selection cancelled

    // Check ending of selected file
    if (!fileName.endsWith(Constants::LoadPage::FileExt, Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, tr(Constants::LoadPage::Errors::InvalidFileTitle),
                             tr(Constants::LoadPage::Errors::InvalidFileBody));
        return;
    }

    showStatusMessage(Constants::LoadPage::StatusParsing, false);
    emit fileSelected(fileName);
}

auto LoadPage::createCardLayout(QVBoxLayout* parentLayout, QWidget* parent) -> QVBoxLayout*
{
    const auto& spacing = THEME.spacing();

    auto* loadCard = new Core::CardWidget(Constants::LoadPage::CardTitle,
                                          Constants::LoadPage::CardSubtitle, "", parent);
    loadCard->setMaximumWidth(spacing.WidthLg);
    loadCard->setMaximumHeight(spacing.HeightXl);
    loadCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    parentLayout->addWidget(loadCard);

    // Inner layout of load card
    auto* cardLayout = loadCard->contentLayout();
    cardLayout->setContentsMargins(spacing.spacingXl, spacing.spacingXl, spacing.spacingXl,
                                   spacing.spacingXl);
    cardLayout->setSpacing(spacing.spacingMd);
    return cardLayout;
}

void LoadPage::setupUploadZone(QVBoxLayout* layout)
{
    const auto& spacing = THEME.spacing();

    // --- Upload-Zone Frame ---
    m_uploadBoxFrame = new QFrame(this);
    m_uploadBoxFrame->setAcceptDrops(true);
    m_uploadBoxFrame->setObjectName(Constants::LoadPage::ObjectName::UploadZone);
    m_uploadBoxFrame->setCursor(Qt::PointingHandCursor);
    m_uploadBoxFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(m_uploadBoxFrame);

    // --- Inner Layout ---
    auto* zoneLayout = new QVBoxLayout(m_uploadBoxFrame);
    zoneLayout->setAlignment(Qt::AlignCenter);

    // Upload Icon + Instruction Text
    m_iconLabel = new Core::TintedIconLabel(Constants::LoadPage::CardIcon, spacing.IconLg,
                                            Qt::black, m_uploadBoxFrame);
    zoneLayout->addWidget(m_iconLabel);

    m_instructionLabel = createUploadInstruction(m_uploadBoxFrame);
    zoneLayout->addWidget(m_instructionLabel);

    // Status label
    m_statusLabel = new QLabel("");
    m_statusLabel->setObjectName("StatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();  // initially invisible
    zoneLayout->addWidget(m_statusLabel);

    // --- EventFilter for Mouse Click ---
    m_uploadBoxFrame->installEventFilter(this);
}
void LoadPage::setupUi()
{
    // Create Page Layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* cardLayout = createCardLayout(mainLayout);
    setupUploadZone(cardLayout);

    applyStyle();
}

void LoadPage::applyStyle() const
{
    const auto& colors = THEME.colors();

    m_uploadBoxFrame->setStyleSheet(Style::LoadPage::uploadZone());

    if (m_instructionLabel)
    {
        m_instructionLabel->setStyleSheet(Style::LoadPage::uploadInstruction());
    }

    if (m_iconLabel)
    {
        m_iconLabel->setColor(colors.textSecondary);
    }
}

auto LoadPage::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace DbcFile