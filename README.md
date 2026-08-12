# Taskmanager-Harbor

**Taskmanager-Harbor** is a modern, high-performance, dark-themed System Monitor & Task Manager for Linux (specifically optimized for Arch Linux), written in **C++20** and **Qt 6**.

Designed to be sleeker, faster, and more detailed than standard Linux system monitors while consuming minimal CPU overhead (**< 0.5% CPU load**).

---

## 🌟 Key Features

- **📊 Overview Dashboard**: Real-time multi-metric sparkline grid for CPU, RAM, Network, and Disk I/O.
- **🚀 Desktop Applications View**:
  - Parses system `.desktop` application files from `/usr/share/applications/` and `~/.local/share/applications/`.
  - Groups child helper processes (e.g. Brave, Discord, Telegram, Dolphin, Konsole, Kate, digiKam, Nextcloud, Taskmanager-Harbor) into single application entries with official XDG desktop icons.
  - Aggregates non-desktop background daemons into a clean **Background Services** row.
  - Displays exact memory consumption in **GiB** and **MiB**.
- **💻 CPU & Cores View**:
  - Overall CPU usage sparkline graph.
  - Per-core and per-thread load meters with real-time frequency display (GHz).
- **🧠 Memory & Swap View**:
  - Detailed breakdown of total, used, and buffered/cached RAM in **GB**.
  - Swap memory visualizer with vibrant gradient sparklines.
- **🌐 Network Traffic View**:
  - Dual-line real-time graph for incoming (RX - Cyan) and outgoing (TX - Pink) network traffic.
  - Per-interface breakdown table (e.g. `eth0`, `wlan0`) with cumulative transfer statistics.
- **💾 Disks & Storage View**:
  - Detects all mounted physical drives (NVMe SSD, SATA SSD/HDD) and removable **USB drives**.
  - **Disk Selector Dropdown**: Switch between *All Disks (Combined Total)* and individual physical drives (*nvme0n1*, *sda*, *sdb*, etc.) to view per-disk real-time Read/Write sparkline graphs.
  - Storage space capacity bars (Used / Available / Total GB & TB).
- **⚡ Interactive Process Manager**:
  - Search filter by process name or PID.
  - Sortable columns (PID, Name, User, CPU %, RAM MB, Status, Nice).
  - Right-click context menu & action buttons to End Task (`SIGTERM`), Force Kill (`SIGKILL`), Pause/Resume (`SIGSTOP`/`SIGCONT`), or adjust priority (`nice`).
  - **Privileged Process Elevation**: Integrated `pkexec` (Polkit) support to safely terminate or kill root/system processes via your desktop's native password prompt.
- **ℹ️ Hardware & System Specs**: OS, Linux Kernel version, Hostname, Architecture, CPU model, and live Uptime counter.
- **🎨 Custom Vector Icons & Responsive Wrapping Tab Bar**:
  - Custom SVG vector icons compiled directly into Qt binary resources (`resources.qrc`).
  - **FlowLayout Tab Bar**: Tabs automatically wrap onto new lines (up to 3 rows) when resizing the window smaller.
- **⚡ Ultra-Low Resource Usage**:
  - **Lazy Tab Updates**: Only updates the currently active tab in real-time, reducing CPU work by >85%.
  - **RAM Icon Caching**: Caches application icons in memory to prevent disk I/O thrashing.
  - **Batch UI Repaints**: Prevents micro-lag during process table population.

---

## 🏗️ Building & Installation

### Prerequisites (Arch Linux)

Install the required build tools and Qt 6 libraries:

```bash
sudo pacman -S gcc cmake ninja qt6-base qt6-svg git
```

### Easy Installation (Recommended)

Run the included `install.sh` script to build the executable, install the custom SVG icon, and register the launcher in your application menu:

```bash
git clone https://github.com/walker42195/Taskmanager-Harbor.git
cd Taskmanager-Harbor

chmod +x install.sh
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
