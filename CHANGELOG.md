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