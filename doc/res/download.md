\page download Downloading CANcept

Three ways to get CANcept, also summarized on the \ref index "start page".

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

`start.sh -h` lists build options (tests, docs, coverage, dev mode).