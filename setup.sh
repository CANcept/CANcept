#!/bin/sh
set -e
mkdir -p /etc/apt/keyrings
curl -fsSL https://raw.githubusercontent.com/CANcept/CANcept/apt-repo/cancept.gpg | tee /etc/apt/keyrings/cancept.gpg > /dev/null
echo "deb [signed-by=/etc/apt/keyrings/cancept.gpg] https://raw.githubusercontent.com/CANcept/CANcept/apt-repo stable main" | tee /etc/apt/sources.list.d/cancept.list
apt-get update
echo "Repository added. Run: sudo apt install cancept"
