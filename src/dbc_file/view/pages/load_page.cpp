//
// Created by Adrian Rupp on 21.01.26.
//
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

#include "core/theme/theme_manager.hpp"
#include "dbc_file/constants.hpp"

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
}  // namespace

void LoadPage::showStatusMessage(const QString& message, const bool isError) const
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    m_statusLabel->setText(message);
    m_statusLabel->show();
    const QString labelStyle =
        "font-weight: %1;"
        "font-size: %2px;"
        "margin-top: %3px;"
        "color: %4;";
    const QString color = isError ? colors.statusError.name() : colors.statusSuccess.name();
    m_statusLabel->setStyleSheet(labelStyle.arg(spacing.fontWeightNormal)
                                     .arg(spacing.fontSizeSm)
                                     .arg(spacing.spacingSm)
                                     .arg(color));
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
    m_isDragValid = false;

    // Check if dragged data contain URLS
    if (event->mimeData()->hasUrls())
    {
        // extract URLS
        const QList<QUrl>& urls = event->mimeData()->urls();

        // Check if only one file is being dragged
        if (urls.size() == 1)
        {
            const QString filePath = urls.first().toLocalFile();

            // Check if file has correct ending
            if (filePath.endsWith(Constants::LoadPage::FileExt, Qt::CaseInsensitive))
            {
                m_isDragValid = true;
            }
        }
    }
    if (m_isDragValid)
    {
        // Update drag style to change upload zone border color to indicate dragged data is valid
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
    if (!filePath.endsWith(Constants::LoadPage::FileExt, Qt::CaseInsensitive))
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

auto LoadPage::createCardFrame(QVBoxLayout* parentLayout) -> QVBoxLayout*
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* loadCard = new QFrame(this);
    loadCard->setObjectName(Constants::LoadPage::ObjectName::LoadCard);
    loadCard->setMaximumWidth(650);
    loadCard->setMaximumHeight(350);
    loadCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    const QString cardStyle = QString(
                                  "#LoadCard { "
                                  "background-color: %1; "
                                  "border: %2px solid %3; "
                                  "border-radius: %4px; "
                                  "}")
                                  .arg(colors.surfaceMain.name())
                                  .arg(spacing.borderThick)
                                  .arg(colors.borderSubtle.name(QColor::HexArgb))
                                  .arg(spacing.radiusSm);
    loadCard->setStyleSheet(cardStyle);
    parentLayout->addWidget(loadCard);

    // Inner layout of load card
    auto* cardLayout = new QVBoxLayout(loadCard);
    cardLayout->setContentsMargins(spacing.spacingXl, spacing.spacingXl, spacing.spacingXl,
                                   spacing.spacingXl);
    cardLayout->setSpacing(spacing.spacingMd);
    return cardLayout;
}

void LoadPage::setupHeader(QVBoxLayout* layout)
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* title = new QLabel(Constants::LoadPage::CardTitle);
    title->setStyleSheet(QString("font-size: %1px;"
                                 "font-weight: %2;"
                                 "color: %3;")
                             .arg(spacing.fontSizeLg)
                             .arg(spacing.fontWeightNormal)
                             .arg(colors.textPrimary.name()));
    layout->addWidget(title);

    // Subtitle "Load a DBC file to analyze its content" below the title in the upper left
    auto* subTitle = new QLabel(Constants::LoadPage::CardSubtitle);
    subTitle->setStyleSheet(QString("font-size: %1px;"
                                    "font-weight: %2;"
                                    "color: %3;")
                                .arg(spacing.fontSizeMd)
                                .arg(spacing.fontWeightNormal)
                                .arg(colors.textSecondary.name()));
    layout->addWidget(subTitle);
}

/**
 * @brief Creates a themed upload icon for the upload zone.
 * @param parent Parent widget.
 * @return QLabel containing the icon.
 *
 * @details Uses QPainter to tint the SVG icon according to the theme. Falls back
 * to text if the icon resource is missing.
 */
auto createUploadIcon(QWidget* parent) -> QLabel*
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* iconLabel = new QLabel(parent);
    iconLabel->setAlignment(Qt::AlignCenter);

    QIcon icon(Constants::LoadPage::CardIcon);
    QPixmap pixmap = icon.pixmap(spacing.IconSize, spacing.IconSize);

    if (!pixmap.isNull())
    {
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), colors.textSecondary);
        painter.end();

        iconLabel->setPixmap(pixmap);
    } else
    {
        // Fallback text if icon resource is missing
        iconLabel->setText(Constants::LoadPage::CardIconFallback);
        iconLabel->setStyleSheet(QString("font-size: %1px; color: %2;")
                                     .arg(spacing.fontSizeLg)
                                     .arg(colors.textSecondary.name()));
    }

    return iconLabel;
}

/**
 * @brief Creates an instruction label for the upload zone.
 * @param parent Parent widget.
 * @return QLabel containing the instruction text.
 */
auto createUploadInstruction(QWidget* parent) -> QLabel*
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* textLabel = new QLabel(parent);
    textLabel->setAlignment(Qt::AlignCenter);
    QString fontStyle = QString("font-size: %1px; font-weight: %2; color: %3;")
                            .arg(spacing.fontSizeSm)
                            .arg(spacing.fontWeightNormal)
                            .arg(colors.textSecondary.name());
    textLabel->setStyleSheet(fontStyle);
    textLabel->setText(Constants::LoadPage::CardInstruction);
    return textLabel;
}
void LoadPage::setupUploadZone(QVBoxLayout* layout)
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // --- Upload-Zone Frame ---
    m_uploadBoxFrame = new QFrame(this);
    m_uploadBoxFrame->setAcceptDrops(true);
    m_uploadBoxFrame->setObjectName(Constants::LoadPage::ObjectName::UploadZone);
    m_uploadBoxFrame->setCursor(Qt::PointingHandCursor);
    m_uploadBoxFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --- StyleSheet with Drag-States ---
    QString zoneStyle = QString(
                            // 1. Default style
                            "#UploadZone { "
                            "  background-color: %1;"
                            "  border: %2px solid %3; "
                            "  border-radius: %4px; "
                            "}"

                            // 2. Hover style
                            "#UploadZone:hover { "
                            "  background-color: %5;"
                            "  border-color: %6; "
                            "}"

                            // 3. Valid drag style
                            "#UploadZone[dragState=\"valid\"] { "
                            "  border: %2px solid %7;"
                            "}"

                            // 4. Invalid drag style
                            "#UploadZone[dragState=\"invalid\"] { "
                            "  border: %2px solid %8;"  // %6 = Rot
                            "}")
                            .arg(colors.surfaceMain.name(QColor::HexArgb))
                            .arg(spacing.borderThick)
                            .arg(colors.borderStrong.name(QColor::HexArgb))
                            .arg(spacing.radiusSm)
                            .arg(colors.surfaceHover.name(QColor::HexArgb))
                            .arg(colors.borderStrong.name(QColor::HexArgb))
                            .arg(colors.statusSuccess.name(QColor::HexArgb))
                            .arg(colors.statusError.name(QColor::HexArgb));

    m_uploadBoxFrame->setStyleSheet(zoneStyle);
    layout->addWidget(m_uploadBoxFrame);

    // --- Inner Layout ---
    auto* zoneLayout = new QVBoxLayout(m_uploadBoxFrame);
    zoneLayout->setAlignment(Qt::AlignCenter);

    // Upload Icon + Instruction Text
    zoneLayout->addWidget(createUploadIcon(m_uploadBoxFrame));
    zoneLayout->addWidget(createUploadInstruction(m_uploadBoxFrame));

    // Status label
    m_statusLabel = new QLabel("");
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

    auto* cardLayout = createCardFrame(mainLayout);
    setupHeader(cardLayout);
    setupUploadZone(cardLayout);
}
}  // namespace DbcFile