//
// Created by Adrian Rupp on 19.01.26.
//
#include "subviews.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QPainter>
#include <QUrl>
#include <QVBoxLayout>

#include "core/theme/theme_manager.hpp"
namespace DbcFile {

LoadPage::LoadPage(QWidget* parent) : QWidget(parent)
{
    // Activate Drag and Drop
    setAcceptDrops(true);
    setupUi();
}
LoadPage::~LoadPage() = default;

/**
 * @brief Forces a style update on the widget based on the new property state.
 * @details Calling unpolish() and polish() is required for Qt to re-evaluate
 * dynamic stylesheet selectors like [dragState="valid"].
 */
static void updateDragStyle(QWidget* widget, const QString& state)
{
    widget->setProperty("dragState", state);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

void LoadPage::dragEnterEvent(QDragEnterEvent* event)
{
    bool isValid = false;
    // Check if dragged data contain URLS
    if (event->mimeData()->hasUrls())
    {
        // extract URLS
        const QList<QUrl>& urls = event->mimeData()->urls();

        if (!urls.isEmpty())
        {
            // Check if only one file is dragged and for right ending
            if (const QString filePath = urls.first().toLocalFile();
                urls.size() == 1 && filePath.endsWith(".dbc", Qt::CaseInsensitive))
            {
                isValid = true;
            }
        }
    }
    if (isValid)
    {
        // Update dragstyle to change upload zone border color to indicate dragged data is valid
        updateDragStyle(m_uploadBoxFrame, "valid");
        event->acceptProposedAction();
    } else
    {
        // Update dragstyle to change upload zone border color to indicate dragged data in invalid
        updateDragStyle(m_uploadBoxFrame, "invalid");
        event->ignore();
    }
}

void LoadPage::dropEvent(QDropEvent* event)
{
    // Return to default style of upload zone
    updateDragStyle(m_uploadBoxFrame, "");
    const auto urls = event->mimeData()->urls();
    // Double check validity of data to be dropped
    if (!urls.isEmpty())
    {
        QString file = urls.first().toLocalFile();
        if (file.endsWith(".dbc", Qt::CaseInsensitive))
        {
            emit fileSelected(file);
        }
    }
    event->acceptProposedAction();
}
void LoadPage::dragLeaveEvent(QDragLeaveEvent* event)
{
    // Change upload box style to default when cursor leaves upload box
    updateDragStyle(m_uploadBoxFrame, "");
    event->accept();
}
auto LoadPage::eventFilter(QObject* watched, QEvent* event) -> bool
{
    // Check if event belongs to upload zone
    if (watched == m_uploadBoxFrame)
    {
        // Check if event is a mouse click
        if (event->type() == QEvent::MouseButtonRelease)
        {
            onBrowseButtonClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
void LoadPage::onBrowseButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose DBC file"), QString(),
                                                    tr("DBC files (*.dbc)"));

    if (!fileName.isEmpty())
    {
        emit fileSelected(fileName);
    }
}

void LoadPage::setupUi()
{
    // Load Theme
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // Create Page Layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Outer frame of the loading card
    auto* loadCard = new QFrame(this);
    loadCard->setObjectName("LoadCard");
    loadCard->setFixedSize(650, 450);
    QString cardStyle = QString(
                            "#LoadCard { "
                            "background-color: %1; "
                            "border: %2px solid %3; "
                            "border-radius: %4px; "
                            "}")
                            .arg(colors.colorPrimary.name())
                            .arg(spacing.borderThin)
                            .arg(colors.borderSubtle.name())
                            .arg(spacing.radiusSm);
    loadCard->setStyleSheet(cardStyle);

    mainLayout->addWidget(loadCard);

    // Inner layout of load card
    auto* cardLayout = new QVBoxLayout(loadCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    cardLayout->setSpacing(15);

    // Title "Upload DBC File" in the left of the load card
    auto* title = new QLabel("Upload DBC File", loadCard);
    title->setStyleSheet(QString("font-size: %1px;"
                                 "font-weight: %2;"
                                 "color: %3;")
                             .arg(spacing.fontSizeMd)
                             .arg(spacing.fontWeightNormal)
                             .arg(colors.textPrimary.name()));
    cardLayout->addWidget(title);

    // Subtitle "Load a DBC file to analyze its content" below the title in the upper left
    auto* subTitle = new QLabel("Load a DBC file to analyze its content", loadCard);
    subTitle->setStyleSheet(QString("font-size: %1px;"
                                    "font-weight: %2;"
                                    "color: %3;")
                                .arg(spacing.fontSizeSm)
                                .arg(spacing.fontWeightNormal)
                                .arg(colors.textSecondary.name()));
    cardLayout->addWidget(subTitle);

    cardLayout->addSpacing(10);

    // The drag and drop zone to upload a file
    m_uploadBoxFrame = new QFrame(loadCard);
    m_uploadBoxFrame->setObjectName("UploadZone");
    m_uploadBoxFrame->setCursor(Qt::PointingHandCursor);
    m_uploadBoxFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString zoneStyle = QString(
                            // Default style of the area
                            "#UploadZone { "
                            "background-color: %1;"
                            "border: %2px solid %3; "
                            "border-radius: %4px; "
                            "}"

                            // Valid drag style
                            "#UploadZone[dragState=\"valid\"] { "
                            "border-color: %5"
                            "}"

                            // Invalid drag style
                            "#UploadZone[dragState=\"invalid\"] { "
                            "border-color: %6"
                            "}"

                            // Mouse hover style (without drag)
                            "#UploadZone:hover { "
                            "background-color: %7;"
                            "border-color: %8; "
                            "}")
                            .arg(colors.colorPrimary.name())
                            .arg(spacing.borderThick)
                            .arg(colors.borderStrong.name())
                            .arg(spacing.radiusSm)
                            .arg(colors.statusSuccess.name())
                            .arg(colors.statusError.name())
                            .arg(colors.colorPrimaryHover.name())
                            .arg(colors.borderSubtle.name());

    m_uploadBoxFrame->setStyleSheet(zoneStyle);
    m_uploadBoxFrame->installEventFilter(this);
    cardLayout->addWidget(m_uploadBoxFrame);

    auto* zoneLayout = new QVBoxLayout(m_uploadBoxFrame);
    zoneLayout->setAlignment(Qt::AlignCenter);

    // Content of upload zone: icon
    auto* iconLabel = new QLabel(m_uploadBoxFrame);
    iconLabel->setAlignment(Qt::AlignCenter);

    QIcon icon(":/icons/upload.svg");
    if (!icon.isNull())
    {
        QPixmap pixmap = icon.pixmap(48, 48);

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), colors.textSecondary);
        painter.end();

        iconLabel->setPixmap(pixmap);
    } else
    {
        // Fallback
        iconLabel->setText("⬆");
        iconLabel->setStyleSheet(
            QString("font-size: 40px; color: %1;").arg(colors.textSecondary.name()));
    }
    zoneLayout->addWidget(iconLabel);

    // Content of upload zone: text
    auto* textLabel = new QLabel(m_uploadBoxFrame);
    textLabel->setAlignment(Qt::AlignCenter);
    QString fontStyle = QString("font-size: %1px; font-weight: %2; color: %3;")
                            .arg(spacing.fontSizeSm)
                            .arg(spacing.fontWeightNormal)
                            .arg(colors.textSecondary.name());
    textLabel->setStyleSheet(fontStyle);
    // add line break
    textLabel->setText("Click to upload or drag and drop a file here<br>DBC file (*.dbc)");
    zoneLayout->addWidget(textLabel);
}

}  // namespace DbcFile