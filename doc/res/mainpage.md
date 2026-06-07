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

## System Previews

### Monitoring Interface
The monitoring module captures incoming CAN frames and uses the loaded DBC configuration to translate hex data into physical values (such as Temperature or RPM).

\image html monitoring_mock.png "Monitoring Dashboard Mockup" width=800px

### Simulation and Sending
Users can define messages to be sent over the bus. The system supports a mode for bit-level composition and a signal-based mode for parameter adjustment via the user interface.

\image html send_mock.png "Sending Dashboard Mockup" width=800px