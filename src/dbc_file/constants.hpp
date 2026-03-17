/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QString>

#include "core/macro/theme.hpp"

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

// Ecus
constexpr int EcuName = 0;
constexpr int EcuTotalSignals = 1;

// --- Message Table Columns (for Messages Page) ---
constexpr int MsgName = 0;
constexpr int MsgId = 1;
constexpr int MsgDlc = 2;
constexpr int MsgSender = 3;
constexpr int MsgSigCount = 4;

// --- Signal Table Columns (for Signals Page) ---
constexpr int SigName = 0;
constexpr int SigMessage = 1;
constexpr int SigStartBit = 2;
constexpr int SigUnit = 3;
constexpr int SigLength = 4;
constexpr int SigMin = 5;
constexpr int SigMax = 6;
constexpr int SigFactor = 7;
constexpr int SigOffset = 8;
constexpr int SigByteOrder = 9;
constexpr int SigValueType = 10;
constexpr int SigReceivers = 11;

// Helper
constexpr int EcuColumnCount = 2;
constexpr int OvColumnCount = 6;
constexpr int MsgColumnCount = 5;
constexpr int SignalColumnCount = 12;
}  // namespace Columns

namespace Headers {
// Signals Page Header
static const QString SigName = "Signal Name";
static const QString SigMessage = "Message";
static const QString SigStartBit = "Start Bit";
static const QString SigUnit = "Unit";
static const QString SigLength = "Length";
static const QString SigRange = "Range";
static const QString SigFactor = "Factor";
static const QString SigOffset = "Offset";
static const QString SigByteOrder = "Byte Order";
static const QString SigType = "Type";
static const QString SigReceivers = "Receivers: ";
static const QString SigMin = "Min";
static const QString SigMax = "Max";

static const QString MsgId = "ID";
static const QString MsgName = "Name";
static const QString MsgSender = "Sender";
static const QString MsgDlc = "DLC";
static const QString MsgSigCount = "Signals";
}  // namespace Headers

// =========================================================================
// 2. SYSTEM & COMPONENT
// =========================================================================
namespace Component {
static const QString TabId = "dbc-tab";
static const QString TabTitle = "DBC File";
static const QString TabIcon = ":/assets/icon/dbc_file/dbc_file_tab.svg";
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

constexpr int INDEX_LOAD = 0;
constexpr int INDEX_OVERVIEW = 1;
constexpr int INDEX_ECUS = 2;
constexpr int INDEX_MESSAGES = 3;
constexpr int INDEX_SIGNALS = 4;

constexpr int STACK_COUNT = 5;

static const QString HoverText = "Load DBC file first";
// Icons
static const QString IconLoadNew = ":/assets/icon/dbc_file/load_new.svg";
static const QString IconOverview = ":/assets/icon/dbc_file/overview.svg";
static const QString IconEcus = ":/assets/icon/dbc_file/ecus.svg";
static const QString IconMessages = ":/assets/icon/dbc_file/messages.svg";
static const QString IconSignals = ":/assets/icon/dbc_file/signals.svg";
}  // namespace Sidebar

// --- Load Page Specifics ---
namespace LoadPage {
static const QString CardTitle = "Upload DBC File";
static const QString CardSubtitle = "Load a DBC file to analyze its content";
static const QString CardInstruction =
    "Click to upload or drag and drop a file here<br>DBC file (*.dbc)";
static const QString CardIconFallback = "⬆";
static const QString CardIcon = ":/assets/icon/dbc_file/upload.svg";
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

static const auto FileInfoTitle = QStringLiteral("File Information");
static const auto FileInfoSubTitle = QStringLiteral("Basic information about the DBC file");
static const auto FileNameTitle = QStringLiteral("Filename: ");
static const auto FileVersionTitle = QStringLiteral("Version: ");
static const auto LabelDefault = QStringLiteral("-");
static const auto EcuStatTitle = QStringLiteral("ECUs");
static const auto MessagesStatTitle = QStringLiteral("Messages");
static const auto SignalsStatTitle = QStringLiteral("Signals");
static const auto OrphansStatTitle = QStringLiteral("Orphan Messages");
static const auto OverviewSuffix = QStringLiteral(" Overview");
static const auto OverviewDescription = QStringLiteral("All defined %1 in the network");

namespace LblNames {
constexpr const char* FileName = "lblFileName";
constexpr const char* Version = "lblVersion";
constexpr const char* EcuCount = "lblEcuCount";
constexpr const char* MsgCount = "lblMessageCount";
constexpr const char* SigCount = "lblSignalCount";
constexpr const char* OrphanCount = "lblOrphanCount";
}  // namespace LblNames
}  // namespace OverviewPage

// --- ECUs Page Specifics ---
namespace EcusPage {
static const QString PageHeaderTitle = "ECU / Control Unit View";
static const QString PageHeaderSubtitle =
    "Detailed overview of all defined control units and their messages";
static const QString SearchbarText = "Search ECU by name...";
static const QString FilterAllText = "All ECUs";
static const QString FilterActive = "Only Sending ECUs";
static const QString FilterPassive = "Only receiving ECUs";
static const QString EmptyLabelText = "No ECUs found";
constexpr int FilterActiveIndex = 1;
}  // namespace EcusPage

// --- Messages Page Specifics ---
namespace MessagesPage {
static const QString PageHeaderTitle = "Message View";
static const QString PageHeaderSubtitle = "All CAN Messages with detailed information";
static const QString SearchbarText = "Search Messages by name";
static const QString FilterAllText = "All Senders";
static const QString DetailTitle = "Message Details: %1";
static const QString DetailSubtitle = "ID: %1    Sender: %2    DLC: %3 bytes";
static const QString DetailTitlePlaceholder = "No Message Selected";
static const QString DetailSubtitlePlaceholder =
    "Select a message from the list above to view signals.";
static const QString DefaultValue = "/";
static const QString NoSignalsIndicator = "No Signals defined";
static const QString EmptyLabelText = "No Messages found";
}  // namespace MessagesPage

// --- Signals Page Specifics ---
namespace SignalsPage {
static const QString PageHeaderTitle = "Signal View";
static const QString PageHeaderSubtitle = "All signals from all messages";
static const QString SearchbarText = "Search signal by name";
static const QString FilterAllText = "All units";
static const QString EmptyLabelText = "No Signals found";

static const QString BigEndIndicator = "Big Endian";
static const QString LittleEndIndicator = "Little Endian";
static const QString UnsignedIndicator = "Unsigned";
static const QString SignedIndicator = "Signed";
static const QString LengthUnit = " Bit";
static const QString DefaultUnit = "/";
}  // namespace SignalsPage
}  // namespace DbcFile::Constants