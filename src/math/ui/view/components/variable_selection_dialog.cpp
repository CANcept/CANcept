#include "variable_selection_dialog.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "math/constants.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "math/ui/view/components/variable_row_widget.hpp"

namespace Math {

VariableSelectionDialog::VariableSelectionDialog(
    Providers providers, const std::vector<VariableBinding>& currentBindings, QWidget* parent)
    : QDialog(parent), m_providers(std::move(providers))
{
    setupUi(currentBindings);
}

void VariableSelectionDialog::setupUi(const std::vector<VariableBinding>& currentBindings)
{
    const auto& spacing = THEME.spacing();

    setWindowTitle("Variables");
    setObjectName("variableDialog");
    setMinimumWidth(Constants::DIALOG_MIN_WIDTH);
    setModal(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_StyledBackground, true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // Scrollable row list
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setMinimumHeight(Constants::SCROLL_MIN_HEIGHT);

    m_scrollContent = new QWidget(m_scrollArea);
    m_scrollContent->setObjectName("varScrollContent");
    m_scrollLayout = new QVBoxLayout(m_scrollContent);
    m_scrollLayout->setContentsMargins(spacing.spacingSm, 0, spacing.spacingSm, 0);
    m_scrollLayout->setSpacing(spacing.spacingXs);
    m_scrollLayout->addStretch();

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea, 1);

    // Pre-populate from existing bindings
    for (const auto& binding : currentBindings)
    {
        addRow(&binding);
    }

    // Bottom bar: Add + Confirm
    auto* bottomBar = new QWidget(this);
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    m_addButton = new QPushButton("Add", bottomBar);
    connect(m_addButton, &QPushButton::clicked, this, [this] { addRow(); });
    bottomLayout->addWidget(m_addButton);

    bottomLayout->addStretch();

    m_confirmButton = new QPushButton("Confirm", bottomBar);
    connect(m_confirmButton, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(m_confirmButton);

    mainLayout->addWidget(bottomBar);

    applyStyle();
}

void VariableSelectionDialog::addRow(const VariableBinding* binding)
{
    auto* row = new VariableRowWidget(m_providers, m_scrollContent);
    if (binding)
    {
        row->setFromBinding(*binding);
    }

    connect(row, &VariableRowWidget::removeRequested, this, [this, row] { removeRow(row); });

    // Insert before the stretch
    const int insertIdx = m_scrollLayout->count() - 1;
    m_scrollLayout->insertWidget(insertIdx, row);
    m_rows.push_back(row);
}

void VariableSelectionDialog::removeRow(VariableRowWidget* row)
{
    m_scrollLayout->removeWidget(row);
    std::erase(m_rows, row);
    row->deleteLater();
}

auto VariableSelectionDialog::resultBindings() const
    -> std::vector<std::pair<VariableBinding, std::size_t>>
{
    std::vector<std::pair<VariableBinding, std::size_t>> result;
    for (std::size_t i = 0; i < m_rows.size(); ++i)
    {
        const auto* row = m_rows[i];
        if (!row->isValid()) continue;

        VariableBinding binding;
        binding.symbol = row->symbol();
        binding.configKey = row->configKey();
        binding.typeIndex = row->typeIndex();
        binding.variable = nullptr;
        result.emplace_back(std::move(binding), i);
    }
    return result;
}

auto VariableSelectionDialog::createVariableForRow(const std::size_t rowIndex) const
    -> std::unique_ptr<IVariable>
{
    if (rowIndex >= m_rows.size()) return nullptr;
    return m_rows[rowIndex]->createVariable();
}

void VariableSelectionDialog::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // Tint button icons to match text color
    const QColor iconColor = colors.textPrimary;
    auto setButtonIcon = [&](QPushButton* btn, const QString& path) {
        QPixmap pixmap(path);
        if (!pixmap.isNull())
        {
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), iconColor);
            painter.end();
            btn->setIcon(QIcon(pixmap));
            btn->setIconSize(QSize(spacing.spacingMd, spacing.spacingMd));
        }
    };

    setButtonIcon(m_addButton, Constants::ADD_VARIABLE_ICON_PATH);
    setButtonIcon(m_confirmButton, Constants::CONFIRM_ICON_PATH);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, colors.surfaceMain);
    this->setPalette(pal);

    // Ensure the scroll area viewport is fully tinted with the background color
    if (auto* viewport = m_scrollArea->viewport())
    {
        m_scrollArea->setAttribute(Qt::WA_TranslucentBackground);
        viewport->setAutoFillBackground(false);
        viewport->setAttribute(Qt::WA_TranslucentBackground);
    }

    setStyleSheet(QString(R"(
        #variableDialog {
            background-color: %1;
            color: %8;
        }
        QScrollArea,
        QScrollArea > QWidget,
        QScrollArea #qt_scrollarea_viewport,
        QWidget#varScrollContent {
            background: transparent;
            background-color: transparent;
            border: none;
        }
        QScrollArea {
            background-color: %1;
            border: none;
        }
        QPushButton {
            background-color: %7;
            color: %8;
            border: none;
            border-radius: %9px;
            padding: %10px %11px;
            font-weight: %12;
            font-size: %13px;
            text-align: center;
        }
        QPushButton:hover {
            background-color: %14;
        }
        QPushButton:pressed {
            background-color: %14;
        }
        QScrollBar:vertical {
            background: %1;
            width: %2px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: %3;
            min-height: %4px;
            border-radius: %5px;
        }
        QScrollBar::handle:vertical:hover {
            background: %6;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )")
                      .arg(colors.surfaceMain.name(QColor::HexArgb))
                      .arg(spacing.WidthXs / 10)
                      .arg(colors.surfaceSecondary.name(QColor::HexArgb))
                      .arg(spacing.HeightSm)
                      .arg(spacing.radiusSm / 2)
                      .arg(colors.surfaceHover.name())
                      .arg(colors.colorPrimary.name())
                      .arg(colors.textPrimary.name())
                      .arg(spacing.radiusSm)
                      .arg(spacing.spacingSm)
                      .arg(spacing.spacingMd)
                      .arg(spacing.fontWeightMedium)
                      .arg(spacing.fontSizeSm)
                      .arg(colors.colorPrimaryHover.name()));
}

bool VariableSelectionDialog::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QDialog::event(event);
}

}  // namespace Math
