# Changelog

All notable changes to this project will be documented in this file.

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