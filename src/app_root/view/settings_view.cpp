#include "settings_view.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <algorithm>

#include "app_root/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_combo_box.hpp"

namespace AppRoot {

auto SettingRenderer<Core::SettingType::Select>::create(const Core::ISetting* setting,
                                                        SettingsModel* model,
                                                        QWidget* parent) -> QWidget*
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();
    const auto placeholder = setting->getPlaceholder();
    const auto settingKey = setting->getKey();

    auto* container = new QWidget(parent);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(spacing.spacingSm);

    auto* label = new QLabel(QString::fromStdString(settingKey.settingId), container);
    QFont labelFont = label->font();
    labelFont.setPointSize(spacing.fontSizeXs);
    labelFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightMedium));
    label->setFont(labelFont);
    label->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    layout->addWidget(label);

    auto* combo = new Core::StyledComboBox(container);
    combo->setPlaceholderText(QString::fromStdString(placeholder));

    auto populateCombo = [settingKey, model, combo]() {
        const auto currentValue = model->getValue(settingKey);

        combo->blockSignals(true);
        combo->clear();

        auto settings = model->getSettingsForComponent(settingKey.componentId);
        const auto it =
            std::ranges::find_if(settings, [&](auto* s) { return s->getKey() == settingKey; });

        if (it == settings.end())
        {
            combo->blockSignals(false);
            return;
        }

        const auto options = model->fetchOptions(*it);

        int selectedIndex = -1;
        int i = 0;
        for (const auto& [value, displayText] : options)
        {
            combo->addItem(QString::fromStdString(displayText), QString::fromStdString(value));
            if (value == currentValue)
            {
                selectedIndex = i;
            }
            ++i;
        }

        if (selectedIndex >= 0)
        {
            combo->setCurrentIndex(selectedIndex);
        }
        combo->blockSignals(false);
    };

    QObject::connect(combo, &Core::StyledComboBox::aboutToShowPopup, container, populateCombo);

    QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), container,
                     [settingKey, model, combo](const int index) {
                         if (index < 0)
                         {
                             return;
                         }
                         const auto value = combo->itemData(index).toString().toStdString();
                         model->setValue(settingKey, value);
                     });

    populateCombo();

    layout->addWidget(combo);
    return container;
}

// Dispatch the correct render defintion based on the type of the setting
auto SettingsView::createSettingWidget(const Core::ISetting* setting, SettingsModel* model,
                                       QWidget* parent) -> QWidget*
{
    switch (setting->getType())
    {
        case Core::SettingType::Select:
            return SettingRenderer<Core::SettingType::Select>::create(setting, model, parent);
    }
    return nullptr;
}

SettingsView::SettingsView(SettingsModel* model, QWidget* parent)
    : QWidget(parent),
      m_model(model),
      m_scrollArea(nullptr),
      m_contentWidget(nullptr),
      m_contentLayout(nullptr),
      m_copyrightLabel(nullptr)
{
    setupUi();
    rebuild();
}

void SettingsView::setupUi()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingXl, spacing.spacingLg, spacing.spacingXl,
                                   spacing.spacingLg);
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_contentWidget = new QWidget(m_scrollArea);
    m_contentWidget->setObjectName("settingsContent");
    m_contentWidget->setStyleSheet("QWidget#settingsContent { background: transparent; }");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(spacing.spacingLg);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea, 1);

    m_copyrightLabel = new QLabel(Constants::COPYRIGHT_TEXT, this);
    QFont copyrightFont = m_copyrightLabel->font();
    copyrightFont.setPointSize(spacing.fontSizeXs);
    m_copyrightLabel->setFont(copyrightFont);
    m_copyrightLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_copyrightLabel);

    applyStyle();
}

void SettingsView::rebuild() const
{
    while (m_contentLayout->count() > 1)
    {
        const QLayoutItem* item = m_contentLayout->takeAt(0);
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }

    for (const auto componentIds = m_model->getComponentIds();
         const auto& componentId : componentIds)
    {
        buildComponentSection(componentId);
    }
}

void SettingsView::buildComponentSection(const std::string& componentId) const
{
    const auto& spacing = THEME.spacing();
    const auto settings = m_model->getSettingsForComponent(componentId);

    if (settings.empty())
    {
        return;
    }

    const auto iconPath = settings.front()->getIcon();
    auto* sectionCard = new Core::CardWidget(QString::fromStdString(componentId), QString(),
                                             QString::fromStdString(iconPath), m_contentWidget);

    auto* innerRow = new QHBoxLayout();
    innerRow->setSpacing(spacing.spacingLg);

    for (auto* setting : settings)
    {
        if (auto* widget = createSettingWidget(setting, m_model, sectionCard))
        {
            innerRow->addWidget(widget);
        }
    }

    sectionCard->contentLayout()->addLayout(innerRow);

    const int stretchIndex = m_contentLayout->count() - 1;
    m_contentLayout->insertWidget(stretchIndex, sectionCard);
}

void SettingsView::applyStyle() const
{
    const auto& colors = THEME.colors();
    m_scrollArea->setStyleSheet(QString("background-color: %1;").arg(colors.surfaceMain.name()));
    m_copyrightLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
}

bool SettingsView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        QTimer::singleShot(0, this, [this]() { rebuild(); });
        return true;
    }
    return QWidget::event(event);
}

}  // namespace AppRoot
