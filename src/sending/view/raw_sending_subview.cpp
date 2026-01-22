#include "raw_sending_subview.hpp"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

RawSendingSubView::RawSendingSubView(QWidget* parent)
    : QWidget(parent),
      m_configGroup(nullptr),
      m_idEditor(nullptr),
      m_dlcSpin(nullptr),
      m_payloadGroup(nullptr),
      m_actionGroup(nullptr),
      m_deviceCombo(nullptr),
      m_sendButton(nullptr)
{
    setupUi();
}

void RawSendingSubView::setupUi()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    mainLayout->setSpacing(spacing.spacingLg);

    // === Configuration Group (ID and DLC) ===
    m_configGroup = new QGroupBox(tr("Frame Configuration"), this);
    auto* configLayout = new QHBoxLayout(m_configGroup);
    configLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                     spacing.spacingMd);
    configLayout->setSpacing(spacing.spacingLg);

    // CAN ID Editor
    auto* idLabel = new QLabel(tr("CAN ID:"), m_configGroup);
    m_idEditor = new QLineEdit(m_configGroup);
    m_idEditor->setPlaceholderText("000");
    m_idEditor->setMaxLength(3);
    m_idEditor->setFixedWidth(80);

    // Hex validator for CAN ID (standard 11-bit: 0x000 - 0x7FF)
    QRegularExpression hexRegex("^[0-9A-Fa-f]{1,3}$");
    auto* idValidator = new QRegularExpressionValidator(hexRegex, m_idEditor);
    m_idEditor->setValidator(idValidator);

    // DLC Spinbox
    auto* dlcLabel = new QLabel(tr("DLC:"), m_configGroup);
    m_dlcSpin = new QSpinBox(m_configGroup);
    m_dlcSpin->setRange(0, 8);
    m_dlcSpin->setValue(8);
    m_dlcSpin->setFixedWidth(60);

    configLayout->addWidget(idLabel);
    configLayout->addWidget(m_idEditor);
    configLayout->addSpacing(spacing.spacingXl);
    configLayout->addWidget(dlcLabel);
    configLayout->addWidget(m_dlcSpin);
    configLayout->addStretch();

    mainLayout->addWidget(m_configGroup);

    // === Payload Group (8 byte editors) ===
    m_payloadGroup = new QGroupBox(tr("Data Payload"), this);
    auto* payloadLayout = new QGridLayout(m_payloadGroup);
    payloadLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                      spacing.spacingMd);
    payloadLayout->setSpacing(spacing.spacingSm);

    // Hex validator for bytes (00 - FF)
    const QRegularExpression byteRegex("^[0-9A-Fa-f]{1,2}$");

    m_byteEditors.reserve(8);
    for (int i = 0; i < 8; ++i)
    {
        auto* byteLabel = new QLabel(QString("B%1:").arg(i), m_payloadGroup);
        auto* byteEdit = new QLineEdit(m_payloadGroup);
        byteEdit->setPlaceholderText("00");
        byteEdit->setMaxLength(2);
        byteEdit->setFixedWidth(50);
        byteEdit->setAlignment(Qt::AlignCenter);

        auto* byteValidator = new QRegularExpressionValidator(byteRegex, byteEdit);
        byteEdit->setValidator(byteValidator);

        m_byteEditors.push_back(byteEdit);

        // Arrange in 2 rows of 4
        int row = i / 4;
        int col = (i % 4) * 2;
        payloadLayout->addWidget(byteLabel, row, col);
        payloadLayout->addWidget(byteEdit, row, col + 1);
    }

    mainLayout->addWidget(m_payloadGroup);

    // === Action Group (Device selector and Send button) ===
    m_actionGroup = new QGroupBox(tr("Transmission"), this);
    auto* actionLayout = new QHBoxLayout(m_actionGroup);
    actionLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                     spacing.spacingMd);
    actionLayout->setSpacing(spacing.spacingLg);

    auto* deviceLabel = new QLabel(tr("Interface:"), m_actionGroup);
    m_deviceCombo = new QComboBox(m_actionGroup);
    m_deviceCombo->setMinimumWidth(120);
    m_deviceCombo->setPlaceholderText(tr("Select interface..."));

    m_sendButton = new QPushButton(tr("Send Message"), m_actionGroup);
    m_sendButton->setMinimumWidth(120);

    actionLayout->addWidget(deviceLabel);
    actionLayout->addWidget(m_deviceCombo);
    actionLayout->addStretch();
    actionLayout->addWidget(m_sendButton);

    mainLayout->addWidget(m_actionGroup);

    // Add stretch at bottom
    mainLayout->addStretch();

    // Connect DLC changes to enable/disable byte editors
    connect(m_dlcSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int dlc) {
        for (int i = 0; i < 8; ++i)
        {
            m_byteEditors[i]->setEnabled(i < dlc);
            if (i >= dlc)
            {
                m_byteEditors[i]->clear();
            }
        }
    });

    // Initialize byte editors based on default DLC
    for (int i = 0; i < 8; ++i)
    {
        m_byteEditors[i]->setEnabled(i < m_dlcSpin->value());
    }
}

void RawSendingSubView::setAvailableDevices(const std::vector<std::string>& devices)
{
    m_deviceCombo->clear();
    for (const auto& device : devices)
    {
        m_deviceCombo->addItem(QString::fromStdString(device));
    }
}

}  // namespace Sending
