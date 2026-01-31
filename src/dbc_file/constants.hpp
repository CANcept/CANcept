//
// Created by Adrian Rupp on 22.01.26.
//
#pragma once

#include <QString>

namespace DbcFile::Constants {

// =========================================================================
// 1. DATA MODEL (Indices & Roles)
// =========================================================================
namespace Columns {
// Overview Row (Metadata)
constexpr int OvFilename = 0;
constexpr int OvVersion = 1;
constexpr int OvEcuCount = 2;
constexpr int OvMsgCount = 3;
constexpr int OvSigCount = 4;
constexpr int OvOrphans = 5;

// Messages
constexpr int MsgName = 0;
constexpr int MsgId = 1;
constexpr int MsgDlc = 2;
constexpr int MsgSender = 3;

// Signals (Detailed View)
constexpr int SigName = 0;
constexpr int SigStartBit = 1;
constexpr int SigLength = 2;
constexpr int SigFactor = 3;
constexpr int SigOffset = 4;
constexpr int SigMin = 5;
constexpr int SigMax = 6;
constexpr int SigUnit = 7;
constexpr int SigByteOrder = 8;
constexpr int SigValueType = 9;
constexpr int SigReceivers = 10;

// Helper
constexpr int TotalCount = 11;
}  // namespace Columns

namespace Headers {
static const QString Name = "Name";
static const QString IdStartBit = "ID / StartBit";
static const QString DlcLength = "DLC / Length [Bit]";
static const QString SenderFactor = "Sender / Factor";
static const QString Offset = "Offset";
static const QString Min = "Min";
static const QString Max = "Max";
static const QString Unit = "Unit";
static const QString ByteOrder = "Byte Order";
static const QString Type = "Type";
static const QString Receivers = "Receiver";
}  // namespace Headers

// =========================================================================
// 2. SYSTEM & COMPONENT
// =========================================================================
namespace Component {
static const QString TabId = "dbc-tab";
static const QString TabTitle = "DBC File";
static const QString TabIcon = ":/assets/icon/dbc_file_tab.svg";
}  // namespace Component

namespace Status {
static const QString ParseSuccess = "File parsed successfully!";
static const QString ErrorPrefix = "Error: ";
}  // namespace Status

// =========================================================================
// 3. UI PAGES & RESOURCES
// =========================================================================

// --- Shared Sidebar Navigation ---
namespace Sidebar {
// Titles
static const QString TitleLoadNew = "Load New";
static const QString TitleOverview = "Overview";
static const QString TitleEcus = "ECUs";
static const QString TitleMessages = "Messages";
static const QString TitleSignals = "Signals";

static const QString HoverText = "Load DBC file first";
// Icons
static const QString IconLoadNew = ":/assets/icon/load_new.svg";
static const QString IconOverview = ":/assets/icon/overview.svg";
static const QString IconEcus = ":/assets/icon/ecus.svg";
static const QString IconMessages = ":/assets/icon/messages.svg";
static const QString IconSignals = ":/assets/icon/signals.svg";
}  // namespace Sidebar

// --- Load Page Specifics ---
namespace LoadPage {
static const QString CardTitle = "Upload DBC File";
static const QString CardSubtitle = "Load a DBC file to analyze its content";
static const QString CardInstruction =
    "Click to upload or drag and drop a file here<br>DBC file (*.dbc)";
static const QString CardIconFallback = "⬆";
static const QString CardIcon = ":/assets/icon/upload.svg";
static const QString StatusParsing = "Parsing...";

static const char* FileDialogTitle = "Choose DBC file";
static const char* FileDialogFilter = "DBC files (*.dbc);;All files (*.*)";
static const QString FileExt = ".dbc";

namespace ObjectName {
constexpr const char* LoadCard = "LoadCard";
constexpr const char* UploadZone = "UploadZone";
}  // namespace ObjectName

namespace Errors {
static const char* TooManyFiles = "Please drop only <b>one</b> DBC file at a time.";
static const char* InvalidFileTitle = "Invalid file";
static const char* InvalidFileBody = "Not a DBC file";
}  // namespace Errors

namespace Drag {
constexpr const char* Property = "dragState";
constexpr const char* Valid = "valid";
constexpr const char* Invalid = "invalid";
constexpr const char* None = "";
}  // namespace Drag
}  // namespace LoadPage

// --- Overview Page Specifics ---
namespace OverviewPage {

static const QString FileInfoTitle =
    QStringLiteral("File Information");

static const QString FileInfoSubTitle =
    QStringLiteral("Basic information about the DBC file");

static const QString FileNameTitle =
    QStringLiteral("Filename: ");

static const QString FileVersionTitle =
    QStringLiteral("Version: ");

static const QString LabelDefault =
    QStringLiteral("-");

static const QString EcuStatTitle =
    QStringLiteral("ECUs");

static const QString MessagesStatTitle =
    QStringLiteral("Messages");

static const QString SignalsStatTitle =
    QStringLiteral("Signals");

static const QString OrphansStatTitle =
    QStringLiteral("Orphan Messages");

static const QString OverviewSuffix = QStringLiteral(" Overview");

static const QString OverviewDescription =
    QStringLiteral("All defined %1 in the network");
} // namespace OverviewPage

// --- ECUs Page Specifics ---
namespace EcusPage {
}

// --- Messages Page Specifics ---
namespace MessagesPage {
}

// --- Signals Page Specifics ---
namespace SignalsPage {
}
}  // namespace DbcFile::Constants
