# Taskmanager-Harbor

**Taskmanager-Harbor** is a modern, high-performance, dark-themed System Monitor & Task Manager for **Linux** and **Windows**, written in **C++** and **Qt 6**.

Designed to be sleeker, faster, and more detailed than standard system monitors while consuming minimal CPU overhead (**< 0.5% CPU load**).

---

## 🌟 Key Features

- **📊 Overview Dashboard**: Real-time multi-metric sparkline grid for CPU, GPU, RAM, Network, and Disk I/O.
- **🎮 GPU & VRAM View**: Real-time core utilization, VRAM usage (GB & %), GPU temperature (°C), power draw (W), and clock speed (MHz) for NVIDIA, AMD, and Intel GPUs.
- **🚀 Desktop Applications View**:
  - Groups child helper processes (e.g. Brave, Discord, Telegram, Taskmanager-Harbor) into single application entries with official desktop icons.
  - Aggregates non-desktop background daemons into a clean **Background Services** row.
  - Displays exact memory consumption in **GB** and **MB**.
- **💻 CPU & Cores View**:
  - Overall CPU usage sparkline graph.
  - Per-core and per-thread load meters with real-time frequency display (GHz).
- **🧠 Memory & Swap View**:
  - Detailed breakdown of total, used, and buffered/cached RAM in **GB**.
  - Swap memory visualizer with vibrant gradient sparklines.
- **🌐 Network Traffic View**:
  - Dual-line real-time graph for incoming (RX) and outgoing (TX) network traffic.
  - Per-interface breakdown table with cumulative transfer statistics.
- **💾 Disks & Storage View**:
  - Detects all mounted physical drives (NVMe SSD, SATA SSD/HDD) and removable **USB drives**.
  - Real-time Read/Write sparkline graphs and capacity bars (Used / Available / Total GB & TB).
- **⚡ Interactive Process Manager**:
  - Search filter by process name or PID.
  - Sortable columns (PID, Name, User, CPU %, RAM MB, Status, Nice).
  - End Task, Force Kill, Pause/Resume, or adjust priority (`nice`).
- **ℹ️ Hardware & System Specs**: OS, Kernel / Windows NT version, Hostname, Architecture, CPU model, and live Uptime counter.

---

## 🪟 Windows Download & Usage

Pre-compiled standalone package for 64-bit Windows (Windows 10 / 11):

- ⚡ **[Download Taskmanager-Harbor-Windows-x64.zip (Direct Download)](https://github.com/walker42195/Taskmanager-Harbor/releases/download/latest/Taskmanager-Harbor-Windows-x64.zip)**
- 🔗 **[GitHub Releases Page](https://github.com/walker42195/Taskmanager-Harbor/releases/tag/latest)**

**Usage**: Simply extract `Taskmanager-Harbor-Windows-x64.zip` anywhere on your PC and launch `Taskmanager-Harbor.exe`. No installation required!

---

## 🐧 Linux Installation & Usage

### Option 1: Quick Install (Pre-compiled Binary)

You can download the pre-compiled ready-to-run release directly from [GitHub Releases](https://github.com/walker42195/Taskmanager-Harbor/releases/latest):

```bash
# 1. Download and extract the latest pre-compiled release
wget https://github.com/walker42195/Taskmanager-Harbor/releases/download/v1.0.0/Taskmanager-Harbor-v1.0.0-x86_64.tar.gz
tar -xzvf Taskmanager-Harbor-v1.0.0-x86_64.tar.gz

# 2. Run the quick installer (installs binary & menu launcher)
./install.sh
```

---

### Option 2: Build from Source (Arch Linux / Generic Linux)

Install build dependencies:

```bash
sudo pacman -S gcc cmake ninja qt6-base qt6-svg git
```

Clone and build:

```bash
git clone https://github.com/walker42195/Taskmanager-Harbor.git
cd Taskmanager-Harbor

./install.sh
```

Once installed, you can launch **Taskmanager-Harbor** directly from your desktop application menu or by running:

```bash
taskmanager-harbor
```

### Manual Build

If you prefer to build manually:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

./build/Taskmanager-Harbor
```

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the [LICENSE](LICENSE) file for details.
