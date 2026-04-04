#include "signal_variable_provider.hpp"

#include <QHBoxLayout>
#include <QLabel>

#include "core/widgets/common/styled_combo_box.hpp"
#include "math/types/variables/i_variable.hpp"

namespace Math {

namespace {

constexpr auto MSG_COMBO_NAME = "msgCombo";
constexpr auto SIG_COMBO_NAME = "sigCombo";

}  // namespace

auto SignalVariableProvider::createOptionsWidget(QWidget* parent) -> QWidget*
{
    auto* container = new QWidget(parent);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto* msgCombo = new Core::StyledComboBox(container);
    msgCombo->setObjectName(MSG_COMBO_NAME);

    auto* sigCombo = new Core::StyledComboBox(container);
    sigCombo->setObjectName(SIG_COMBO_NAME);

    if (!m_dbcConfig)
    {
        auto* label = new QLabel("No DBC loaded", container);
        layout->addWidget(label);
        msgCombo->setVisible(false);
        sigCombo->setVisible(false);
        layout->addWidget(msgCombo);
        layout->addWidget(sigCombo);
        return container;
    }

    // Populate message dropdown
    for (const auto& msg : m_dbcConfig->messageDefinitions)
    {
        const auto displayText = QString::fromStdString(msg.messageName) + " (0x" +
                                 QString::number(msg.messageId, 16).toUpper() + ")";
        msgCombo->addItem(displayText, msg.messageId);
    }

    const Core::DbcConfig* cfg = m_dbcConfig;
    QObject::connect(msgCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), container,
                     [cfg, sigCombo, msgCombo](const int index) {
                         sigCombo->clear();
                         if (index < 0 || !cfg) return;

                         const uint32_t msgId = msgCombo->itemData(index).toUInt();
                         for (const auto& msg : cfg->messageDefinitions)
                         {
                             if (msg.messageId == msgId)
                             {
                                 for (const auto& sig : msg.signalDescriptions)
                                 {
                                     sigCombo->addItem(QString::fromStdString(sig.signalName));
                                 }
                                 break;
                             }
                         }
                     });

    layout->addWidget(msgCombo);
    layout->addWidget(sigCombo);

    // Trigger initial signal population
    if (msgCombo->count() > 0)
    {
        msgCombo->setCurrentIndex(0);
        emit msgCombo->currentIndexChanged(0);
    }

    return container;
}

auto SignalVariableProvider::findCombos(const QWidget* optionsWidget)
    -> std::pair<const QComboBox*, const QComboBox*>
{
    const auto* msgCombo = optionsWidget->findChild<const QComboBox*>(MSG_COMBO_NAME);
    const auto* sigCombo = optionsWidget->findChild<const QComboBox*>(SIG_COMBO_NAME);
    return {msgCombo, sigCombo};
}

void SignalVariableProvider::restoreFromVariable(QWidget* optionsWidget,
                                                 const IVariable* variable) const
{
    if (!optionsWidget || !variable) return;

    const auto* sig = dynamic_cast<const CanSignalVariable*>(variable);
    if (!sig) return;

    auto* msgCombo = optionsWidget->findChild<QComboBox*>(MSG_COMBO_NAME);
    auto* sigCombo = optionsWidget->findChild<QComboBox*>(SIG_COMBO_NAME);
    if (!msgCombo || !sigCombo) return;

    // Find and select the matching message
    for (int i = 0; i < msgCombo->count(); ++i)
    {
        if (msgCombo->itemData(i).toUInt() == sig->messageId())
        {
            msgCombo->setCurrentIndex(i);
            break;
        }
    }

    // Find and select the matching signal
    const auto sigName = QString::fromStdString(sig->signalName());
    for (int i = 0; i < sigCombo->count(); ++i)
    {
        if (sigCombo->itemText(i) == sigName)
        {
            sigCombo->setCurrentIndex(i);
            break;
        }
    }
}

auto SignalVariableProvider::configKey(const QWidget* optionsWidget) const -> std::string
{
    const auto [msgCombo, sigCombo] = findCombos(optionsWidget);
    if (!msgCombo || !sigCombo || msgCombo->currentIndex() < 0 || sigCombo->currentIndex() < 0)
    {
        return {};
    }

    const uint32_t msgId = msgCombo->currentData().toUInt();
    const std::string sigName = sigCombo->currentText().toStdString();
    return "signal:" + std::to_string(msgId) + ":" + sigName;
}

auto SignalVariableProvider::displayName(const QWidget* optionsWidget) const -> std::string
{
    const auto [msgCombo, sigCombo] = findCombos(optionsWidget);
    if (!msgCombo || !sigCombo || msgCombo->currentIndex() < 0 || sigCombo->currentIndex() < 0)
    {
        return "Signal";
    }

    const std::string msgDisplay = msgCombo->currentText().toStdString();
    const auto parenPos = msgDisplay.find(" (0x");
    const std::string msgName =
        parenPos != std::string::npos ? msgDisplay.substr(0, parenPos) : msgDisplay;
    const std::string sigName = sigCombo->currentText().toStdString();
    return msgName + "." + sigName;
}

auto SignalVariableProvider::createVariable(const QWidget* optionsWidget) const
    -> std::unique_ptr<IVariable>
{
    const auto [msgCombo, sigCombo] = findCombos(optionsWidget);
    if (!msgCombo || !sigCombo || msgCombo->currentIndex() < 0 || sigCombo->currentIndex() < 0)
    {
        return nullptr;
    }

    const uint32_t msgId = msgCombo->currentData().toUInt();
    const std::string sigName = sigCombo->currentText().toStdString();

    const std::string msgDisplay = msgCombo->currentText().toStdString();
    const auto parenPos = msgDisplay.find(" (0x");
    const std::string msgName =
        parenPos != std::string::npos ? msgDisplay.substr(0, parenPos) : msgDisplay;

    return std::make_unique<CanSignalVariable>(msgId, sigName, msgName);
}

}  // namespace Math
