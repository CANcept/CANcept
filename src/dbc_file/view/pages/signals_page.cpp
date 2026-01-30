//
// Created by Adrian Rupp on 22.01.26.
//
#include "signals_page.hpp"

#include <qstandarditemmodel.h>

#include <QListView>
#include <QVBoxLayout>

#include "core/ui/delegates/card_list_delegate.hpp"
#include "core/ui/widgets/section_header.hpp"
#include "dbc_file/constants.hpp"
namespace DbcFile {
// --- SignalsPage Dummy ---
SignalsPage::SignalsPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
void SignalsPage::setModel(QAbstractItemModel* model) {}
auto SignalsPage::getFilterCombo() const -> QComboBox*
{
    return nullptr;
}
void SignalsPage::setupUi()
{

    enum Roles
    {
        // Diese rollen müssen erstellt werden und die werte aus der dbcConfig geholt werden und das muss dann zusammen ins model gegeben werden
        SignalsCount_Role = Qt::UserRole + 1,
        Unit_Role = Qt::UserRole + 2,
        Range_Role = Qt::UserRole + 3,

    };
    auto* layout = new QVBoxLayout(this);

    // =========================================================
    // BEISPIEL 1: MESSAGES (Kacheln mit Zähler & Icon Badge)
    // =========================================================
    auto* label1 = new QLabel("Example 1: Messages (Icon Mode)", this);
    layout->addWidget(label1);

    auto* msgList = new QListView(this);
    msgList->setViewMode(QListView::IconMode); // Nebeneinander
    msgList->setResizeMode(QListView::Adjust);
    msgList->setFixedHeight(150); // Platz für Test

    auto* msgModel = new QStandardItemModel(this);

    // Message 1
    auto* m1 = new QStandardItem("EngineData");
    m1->setData(QIcon(Constants::Sidebar::IconMessages), Qt::DecorationRole); // Haupt Icon als Qt::DecorationRole im model speichern
    m1->setData("4", SignalsCount_Role); // Badge: Anzahl Signale
    msgModel->appendRow(m1);

    // Message 2
    auto* m2 = new QStandardItem("BrakeStatus");
    m2->setData(QIcon(":/icons/msg.svg"), Qt::DecorationRole);
    m2->setData("2", SignalsCount_Role);
    msgModel->appendRow(m2);

    msgList->setModel(msgModel);

    // Delegate Config: Badge Text aus UserRole+1, Badge Icon fix "Wave"
    auto* msgDelegate = new Core::CardListDelegate(
        SignalsCount_Role,           // Badge Rolle übergeben
        QIcon(Constants::Sidebar::IconSignals),  // Badge Icon (Signale) übergeben
        -1,                         // Kein Detail Text: -1 übergeben
        this
    );
    msgList->setItemDelegate(msgDelegate);
    layout->addWidget(msgList);


    // =========================================================
    // BEISPIEL 2: SIGNALS (Liste mit Range & Unit)
    // =========================================================
    auto* label2 = new QLabel("Example 2: Signals (Icon Mode)", this);
    layout->addWidget(label2);

    auto* sigList = new QListView(this);

    // WICHTIG: IconMode für horizontale Anordnung
    sigList->setViewMode(QListView::IconMode);
    sigList->setResizeMode(QListView::Adjust);
    sigList->setWrapping(true);
    sigList->setUniformItemSizes(true); // Performance & gleiches Layout
    sigList->setFixedHeight(150);

    auto* sigModel = new QStandardItemModel(this);

    // Signal 1
    auto* s1 = new QStandardItem("EngineSpeed");
    s1->setData(QIcon(Constants::Sidebar::IconSignals), Qt::DecorationRole);
    s1->setData("rpm", Unit_Role);       // Badge: Unit
    s1->setData("[0, 8000]", Range_Role); // Detail: Range
    sigModel->appendRow(s1);

    // Signal 2
    auto* s2 = new QStandardItem("Temperature");
    s2->setData(QIcon(Constants::Sidebar::IconSignals), Qt::DecorationRole);
    s2->setData("°C", Unit_Role);
    s2->setData("[-40, 150]", Range_Role);
    sigModel->appendRow(s2);

    sigList->setModel(sigModel);

    // Delegate Config (Badge=Role+1, Detail=Role+2, Kein BadgeIcon)
    auto* sigDelegate = new Core::CardListDelegate(
        Unit_Role,
        QIcon(),        //leeres icon übergeben wenn kein icon im badge gewünscht
        Range_Role,
        this
    );
    sigList->setItemDelegate(sigDelegate);
    layout->addWidget(sigList);

};
}  // namespace DbcFile