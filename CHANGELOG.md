# Changelog

All notable changes to this project will be documented in this file.
## [1.1.0] - 2026-03-14

### Added
#### Tests
 - Added unit tests for all application modules covering all core logic with high code coverage, including edge cases and error handling scenarios.
 - Added integration tests for critical user flows across modules, including DBC file loading, message sending.
 - Added system tests for end-to-end application behavior, including UI interactions and theme switching.
 - Added performance tests to ensure application speed under heavy load and with large DBC files.
 - Implemented test utilities for mocking app components, events, and CAN interactions to facilitate testing.

### Fixed

#### General

- Hex decimal numbers for the message ids are represented with a small x in 0x... (DBCView, MonitoringView, Sending)
- Resolved a segmentation fault caused by redundant ownership of module main views.
- Corrected an incorrect color gradient rendering specifically within the Dracula theme.


#### App Root

- Patched a segmentation fault in the app root shutdown logic triggered by double-invocation.


#### DBC File

- Search texts and selected filters are now reset after a new DBC file is loaded in the DBC File Tab, preventing items being filtered out based on filters set for the previously loaded file.
- Some hidden logic functions within the DBC-File Module have been outsourced into dedicated util files (e.g. unit-, sender-extraction logic for filtering, file validation logic).
- ECU items in the ECUs section are no longer filtered out when children of the ECU do not match the filter.


#### Sending

- Fixed a display error where the repeated sending range was rendered incorrectly.
- Fixed timing discrepancies in the repeated sending module and improved scheduler granularity.
- Fixed a UI regression in the Sending tab where input padding broke after a theme change.


#### Monitoring

- Hex Decimal numbers are correctly converted in the MonitoringView
- The GraphList in the Monitoring tab is correctly aligned to the top.
- Removed redundant row counter.
- Fixed a concurrency issue in the monitoring model.
- Fixed a possible segmentation fault in the monitoring model.


#### Logging

- Fixed a bug where the start button icon color failed to update during a theme switch
- Fixed a concurrency issue in the logging model.
- Refactored the monitoring model and component to be more easily usable.


#### Can Handler

- Fixed a bug when parsing CAN messages with big endian values.
- Made FileParser also parse CRLF file endings correctly.
- Fixed a bug that made the DBC parser parse some files not correctly.


## [1.0.0] - 2026-02-16

### Added

#### Logging Module (CBS-93)
- **Complete logging functionality** for CAN bus messages
  - DBC-based logging with efficient signal-specific data storage
  - Raw CAN message logging support
  - Session management with persistent storage
  - Log export to CSV format
  - Session history table with details view
  - Real-time timer display during recording
  - Message and signal selection dialog
  - Custom delegate for enhanced table rendering
  - Themed UI components matching application style
  - Start/Stop button with visual state indication

#### DBC File Module Improvements (CBS-117)
- **Enhanced DBC file viewer** with better UX
  - Added "No ECUs", "No Signals", and "No Messages" empty state indicators
  - Improved ID and number formatting with dedicated utilities (`dbc_utils`)
  - Centralized style sheets in `styles.cpp` for consistency
  - Styled scrollbars with uniform design
  - Theme-aware styling with `applyStyle()` functions
  - Better searchable filter widgets

#### Sending Module Updates (CBS-119)
- **Sending module enhancements**
  - Updated view and event handling
  - Device readiness overlay system
  - Improved load handling

#### Monitoring Module Improvements (CBS-121, CBS-122)
- **Enhanced monitoring interface**
  - Device not configured overlay with settings icon
  - DBC file selection label
  - Improved styling and theme integration
  - Compact signal list design with reduced padding
  - Better visual consistency

#### DBC Module UX (CBS-123)
- **Better user feedback** for empty states
  - "No Messages/Signals found" indicators in messages and signals pages
  - Styled scrollbar for master table
  - Code formatting and documentation improvements

### Fixed

#### DBC File Module
- Fixed clicking bug in table selection (CBS-117)
- Fixed orphan count not appearing bug (CBS-117)
- Fixed missing data for orphan messages (CBS-117)
- Fixed visual bugs and padding issues (CBS-117)

#### Monitoring Module
- Fixed overlay display issues (CBS-121)
- Fixed graph theme updater bug (CBS-122)

### Changed

#### DBC File Module
- Refactored searchable filter widgets for better maintainability (CBS-117)
- Moved all DBC file style sheets to centralized `styles.cpp` (CBS-117)
- Improved delegate rendering for better visual consistency (CBS-117)

#### Application-Wide
- Unified scrollbar styling across all modules
- Consistent theme application with `applyStyle()` pattern
- Better empty state handling across all views
- Compact UI elements with optimized spacing

---

## [0.2.0] - 2026-02-13

### Added

#### Monitoring Module
- **Live Monitoring** - Real-time CAN frame tracking with graph visualization and auto-pruning (60s window) (CBS-88, CBS-103).
- **Signal Tracking** - Added minimum and maximum value roles for monitored signals (CBS-103).

#### Sending Module
- **Cyclic Transmission** - Added repeated sending functionality with configurable intervals (CBS-116).
- **Interface Selection** - Improved mechanism for selecting CAN interfaces (CBS-106).

#### DBC Module
- **Extended Support** - Added handling for ECUs (CBS-111), Signals/Messages (CBS-115), and Big Endian formats (CBS-114).
- **Overview Page** - New dashboard view for file statistics and summary (CBS-103, CBS-104).
- **Config Persistence** - Implemented configuration management for DBC settings (CBS-113).

#### Core & UI
- **Device Discovery** - Event-driven architecture for querying available CAN hardware (CBS-105).
- **UI Components** - Added `CardListDelegate`, `ItemPainter`, and `TintedIconLabel` for complex list rendering (CBS-104).

### Changed
- **Refactoring** - Restructured `core/ui` into dedicated subdirectories (widgets, delegates, painters) (CBS-104).
- **Sidebar** - Outsourced tab creation to `SidebarDelegate` for better separation of concerns (CBS-98).
- **Theming** - Updated spacing and color themes with new card and badge metrics (CBS-104).

### Fixed
- **Stability** - Resolved deadlocks, memory leaks, and race conditions in `CanDbcHandler` and `CanCommunicationHandler` (CBS-95).
- **Crashes** - Fixed segmentation faults during DBC loading, parsing, and rapid toggle interactions (CBS-88, CBS-96).
- **Parser Logic** - Corrected regex errors and whitespace handling in DBC file parsing (CBS-96).

---

## [0.1.0] - 2026-02-02

### Added

#### Application Core
- **AppRoot** - Main application window with tab-based navigation and composition root architecture (CBS-80, CBS-66)
- **EventBroker** - Central event bus using EnTT dispatcher for type-safe event publishing and subscribing with RAII connection handles (CBS-82)
- **Theme System** - Color and spacing themes with ThemeManager for consistent UI styling

#### CAN Communication
- **CanCommunicationHandler** - Core CAN hardware interface with lifecycle management (CBS-81)
- **CanDeviceHandler** - CAN device discovery and management with message reading/sending capabilities (CBS-81, CBS-105)
- **CanRawHandler** - Raw CAN frame transmission and reception (CBS-81)
- **CanDbcHandler** - DBC-based CAN message encoding/decoding with thread-safe operations (CBS-81, CBS-95)
- **GetAvailableCanDevicesEvent** - Event for discovering available CAN interfaces (CBS-105)

#### DBC File Handling
- **DbcParser** - Full DBC file parser supporting messages, signals, ECUs, comments, and value descriptions (CBS-96)
- **DbcHandler** - High-level DBC configuration management with event-driven updates (CBS-58, CBS-96)
- **FileParser** - File reading utilities for DBC parsing (CBS-96)

#### DBC File Module (UI)
- **DbcComponent** - Tab component for DBC file management with Model-View-Delegate pattern (CBS-98)
- **DbcModel** - Data model for DBC items with Qt model/view integration (CBS-89)
- **DbcView** - Main view with sidebar navigation and stacked pages (CBS-98)
- **LoadPage** - Drag-and-drop file upload zone with visual feedback and validation (CBS-98, CBS-99)

#### Sending Module
- **SendingComponent** - Tab component for CAN message transmission with threading model (CBS-84)
- **SendingModel** - Data model managing raw and DBC-based sending state (CBS-84)
- **SendingView** - Main view with sidebar navigation between raw and DBC-based sending (CBS-84)
- **RawSendingSubview** - UI for sending raw CAN frames with hex ID and data input (CBS-84)
- **DbcBasedSendingSubview** - UI for sending DBC-defined messages with signal value inputs (CBS-84)

#### Core Widgets
- **CardWidget** - Reusable styled card container
- **StyledCheckbox** - Theme-aware checkbox component
- **StyledComboBox** - Theme-aware dropdown component
- **StyledLineEdit** - Theme-aware text input component
- **DbcMessageCard** - Widget for displaying DBC message details with signals
- **DbcSignalRow** - Widget for displaying and editing individual signal values

### Changed
- DBC config DTOs restructured to better reflect DBC file structure (CBS-81)
- Time values in CAN DTOs updated for consistency (CBS-81)
- DBC config storage optimized for performance (CBS-81)