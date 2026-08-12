# ⛵ Taskmanager-Harbor

**Taskmanager-Harbor** is a modern, high-performance, dark-themed System Monitor & Task Manager for Linux (specifically optimized for Arch Linux), written in **C++20** and **Qt 6**.

Designed to be sleeker, faster, and more detailed than standard Linux system monitors while consuming minimal CPU overhead.

---

## 🌟 Key Features

- **📊 Real-time Sparkline Graphs**: Smooth, anti-aliased Bézier curve sparklines with glowing gradient fills for CPU, Memory, Network, and Disk I/O.
- **💻 Per-Core CPU Breakdown**: Visualizes overall CPU load and individual core/thread usage bars and frequencies.
- **🧠 Memory & Swap Visualizer**: Dynamic breakdown of total, used, cached/buffer RAM and Swap memory.
- **🌐 Network Traffic (RX / TX)**: Dual-line real-time graph for incoming (Download) and outgoing (Upload) traffic with per-interface breakdown (eth0, wlan0, etc.).
- **⚡ Interactive Process Manager**:
  - Live search filter by process name or PID.
  - Sortable columns (PID, Name, User, CPU %, RAM MB, Status, Nice).
  - Right-click context menu & action buttons to End Task (`SIGTERM`), Force Kill (`SIGKILL`), Pause/Resume (`SIGSTOP`/`SIGCONT`), or adjust priority (`nice`).
- **ℹ️ Hardware & System Specs**: Linux Kernel version, Arch Linux status, Hostname, Uptime counter, and CPU details.
- **🎨 Sleek Dark Theme**: Deep slate/dark navy palette (`#0d0f17`) with vibrant electric cyan, emerald green, neon pink, and orange accents.

---

## 🏗️ Build & Installation (Arch Linux / Generic Linux)

### Dependencies

- C++20 compiler (`g++` or `clang++`)
- `cmake` (>= 3.16)
- `ninja`
- `qt6-base` (Qt6 Widgets)

On Arch Linux:
```bash
sudo pacman -S gcc cmake ninja qt6-base git
```

### Building from Source

```bash
git clone https.github.com/walker42195/Taskmanager-Harbor.git
cd Taskmanager-Harbor

cmake -B build -G Ninja
cmake --build build
```

### Running

```bash
./build/Taskmanager-Harbor
```

---

## 📄 License

MIT License.
