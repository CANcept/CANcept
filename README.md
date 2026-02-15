# CAN Bus Simulator

[![C++ Qt CI](https://github.com/Can-Bus-Manager/Can-Bus-Manager/actions/workflows/ci.yml/badge.svg)](https://github.com/Can-Bus-Manager/Can-Bus-Manager/actions/workflows/ci.yml)
[![Qt 6.7.3+](https://img.shields.io/badge/Qt-6.7.3+-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![C++20](https://img.shields.io/badge/C++-20-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)

A linux desktop application for CAN (Controller Area Network) bus communication, featuring real-time signal monitoring, message transmission, DBC file support, and comprehensive logging capabilities.

## Preview

<p align="center">
  <img src="doc/res/images/monitoring_mock.png" alt="Frame Monitoring" width="45%">
  <img src="doc/res/images/send_mock.png" alt="Send CAN Signals" width="45%">
</p>

## Requirements

| Component | Version                            |
|-----------|------------------------------------|
| CMake | 3.23+                              |
| C++ Compiler | C++20 support (GCC 10+, Clang 12+) |
| Qt | 6.7.3+                             |
| Ninja | Latest (recommended)               |

### Dependencies

The following dependencies are managed as Git submodules:
- [EnTT](https://github.com/skypjack/entt) - Event dispatcher
- [spdlog](https://github.com/gabime/spdlog) - Logging library
- [libsockcanpp](https://github.com/SimonCaworr/libsockcanpp) - CAN socket interface
- [Qwt](https://qwt.sourceforge.io/) - Qt widgets for technical applications
- [GoogleTest](https://github.com/google/googletest) - Testing framework
- [RapidCSV](https://github.com/d99kris/rapidcsv) - CSV parsing

## Installation

### Ubuntu / Debian

```bash
# Install all dependencies
sudo apt update && sudo apt install -y \
            qt6-base-dev \
            qt6-tools-dev \
            qt6-tools-dev-tools \
            libqt6svg6-dev \
            cmake \
            ninja-build \
            build-essential \
            clang-tidy \
            clang-format \
            doxygen \
            graphviz \
            lcov

# Clone the repository
git clone --recursive https://github.com/Can-Bus-Manager/Can-Bus-Manager.git
cd Can-Bus-Manager

# Build and run
chmod +x start.sh
./start.sh
```
## Usage

### Build Commands

| Command | Description |
|---------|-------------|
| `./start.sh` | Standard build (Release + tests + docs) |
| `./start.sh -d` | Development mode (Debug + Ninja) |
| `./start.sh -c` | Clean rebuild |
| `./start.sh -nt -nd` | Quick build (skip tests and docs) |
| `./start.sh -cov` | Generate coverage report |

### Setting Up Virtual CAN

```bash
# Load the vcan kernel module
sudo modprobe vcan

# Create a virtual CAN interface
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

### Project Structure

```
src/
├── app_root/       # Application kernel and main window
├── core/           # Shared interfaces, events, DTOs, utilities
├── event_broker/   # EnTT-based event broker implementation
├── can_handler/    # CAN communication layer
├── dbc_file/       # DBC file management tab
├── logging/        # Message logging tab
├── monitoring/     # Signal monitoring tab
└── sending/        # CAN message sending tab

tests/
├── unit/           # Unit tests
├── integration/    # Integration tests
├── system/         # System tests
└── performance/    # Performance benchmarks
```

## Development

### Code Style

This project uses Google C++ style with the following modifications:
- 4-space indentation
- 100-character line limit
- Braces on new lines

Run the formatter before committing:

```bash
cmake --build build --target format
```

### Branching Strategy

| Branch | Purpose |
|--------|---------|
| `main` | Stable releases only |
| `develop` | Active development |
| `CBS-[number]-[description]` | Feature branches |

**Commit Naming:** `CBS-[number] feat|fix|chore: [description]`

### Documentation

API documentation is generated with Doxygen. After building, open `doc/html/index.html` in your browser.

## Acknowledgments

- Built with [Qt](https://www.qt.io/) framework
- Event system powered by [EnTT](https://github.com/skypjack/entt)
- Plotting widgets by [Qwt](https://qwt.sourceforge.io/)
