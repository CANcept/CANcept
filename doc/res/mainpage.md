# CANcept

## Purpose of the Project
This software was developed to provide a high-performance solution for monitoring and simulating Controller Area Network (CAN) traffic. The primary goal is to allow users to decode raw bus data using DBC databases and to simulate Electronic Control Units (ECUs) by sending cyclic or manual messages.

We chose this architectural approach to ensure that the time-critical nature of CAN communication is handled in a background layer, while the user interface remains responsive and intuitive for diagnostics.

## Participants 
This project is part of the "Praxis der Softwareentwicklung" (PSE) at the Karlsruhe Institute of Technology (KIT), Winter Term 2025/2026.

- Adrian Rupp
- Florian Fehrle
- Junes Sheikhi
- Lino Wertz
- Nele Spatzier

## Installation

### Ubuntu / Debian

```bash
curl -fsSL https://raw.githubusercontent.com/CANcept/CANcept/apt-repo/setup.sh | sudo bash
sudo apt install CANcept
```

### Arch Linux (AUR)

```bash
yay -S cancept
```

### Build from Source

```bash
git clone --recursive https://github.com/CANcept/CANcept.git
cd CANcept
chmod +x start.sh && ./start.sh
```

## System Previews

### DBC File
Loaded DBC databases are browsable per message and signal, showing start bit, length, byte order, factor/offset and physical range for every signal. See \subpage dbc_format for the file format this view is built from.

\image html dbc_overview.png "DBC Message and Signal Browser" width=800px

### Monitoring Interface
The monitoring module captures incoming CAN frames and uses the loaded DBC configuration to translate raw bytes into physical values (such as Protocol or TargetVoltage), plotted live per signal.

\image html trace_monitoring.png "Monitoring Dashboard" width=800px

### Simulation and Sending
Users can define messages to be sent over the bus, including formula-based signal composition and DBC-based manipulations (triggers, effects and mutations) that alter outgoing signal values on the fly.

\image html trace_generation.png "Sending Dashboard with DBC Manipulation" width=800px

### Logging
Recorded sessions are stored per run with session metadata (ID, timestamp, duration, message count) and a scrollable table of decoded signal values over time.

\image html logging_detail.png "Logging Session Detail View" width=800px

## Further Reading

- \subpage download
- \subpage dbc_format
- \subpage model_json
- \subpage testing