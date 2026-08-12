#include "MetricsCollector.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <pwd.h>

namespace fs = std::filesystem;

namespace Harbor {

MetricsCollector::MetricsCollector(QObject *parent)
    : QObject(parent)
{
    scanDesktopEntries();
    readStaticSystemInfo();
    m_elapsed.start();

    connect(&m_timer, &QTimer::timeout, this, &MetricsCollector::updateAll);
    m_timer.start(1000); // 1-second refresh rate default
    updateAll();
}

void MetricsCollector::setUpdateInterval(int ms) {
    m_timer.setInterval(ms);
}

void MetricsCollector::updateAll() {
    double elapsedSec = m_elapsed.restart() / 1000.0;
    if (elapsedSec <= 0.001) elapsedSec = 1.0;
    m_lastSampleIntervalSec = elapsedSec;

    readCpu();
    readMemory();
    readNetwork();
    readDisk();
    readProcesses();
    readApplicationGroups();

    if (!m_sysInfo.cpuModel.empty()) {
        m_cpu.modelName = m_sysInfo.cpuModel;
    }
    m_sysInfo.totalMemoryBytes = m_memory.totalRamBytes;

    m_firstRun = false;
    emit metricsUpdated();
}

void MetricsCollector::readCpu() {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return;

    std::string line;
    std::vector<CoreTime> newCoreTimes;

    while (std::getline(file, line)) {
        if (line.rfind("cpu", 0) != 0) continue; // Only read cpu, cpu0, cpu1...

        std::istringstream ss(line);
        std::string cpuLabel;
        CoreTime ct;
        ss >> cpuLabel >> ct.user >> ct.nice >> ct.system >> ct.idle
           >> ct.iowait >> ct.irq >> ct.softirq >> ct.steal;

        if (cpuLabel == "cpu") {
            if (!m_firstRun && m_cpu.lastTotalTime.total() > 0) {
                uint64_t totalDiff = ct.total() - m_cpu.lastTotalTime.total();
                uint64_t activeDiff = ct.active() - m_cpu.lastTotalTime.active();
                if (totalDiff > 0) {
                    m_cpu.totalUsagePercent = (100.0 * activeDiff) / totalDiff;
                }
            }
            m_cpu.lastTotalTime = ct;
        } else {
            newCoreTimes.push_back(ct);
        }
    }

    if (m_cpu.perCoreUsagePercent.size() != newCoreTimes.size()) {
        m_cpu.perCoreUsagePercent.resize(newCoreTimes.size(), 0.0);
    }

    if (!m_firstRun && m_cpu.lastCoreTimes.size() == newCoreTimes.size()) {
        for (size_t i = 0; i < newCoreTimes.size(); ++i) {
            uint64_t totalDiff = newCoreTimes[i].total() - m_cpu.lastCoreTimes[i].total();
            uint64_t activeDiff = newCoreTimes[i].active() - m_cpu.lastCoreTimes[i].active();
            if (totalDiff > 0) {
                m_cpu.perCoreUsagePercent[i] = (100.0 * activeDiff) / totalDiff;
            }
        }
    }
    m_cpu.lastCoreTimes = newCoreTimes;

    // Read current CPU frequency if available
    std::ifstream freqFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (freqFile.is_open()) {
        double khz = 0;
        if (freqFile >> khz) {
            m_cpu.currentFreqMHz = khz / 1000.0;
        }
    }
}

void MetricsCollector::readMemory() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return;

    std::string line;
    uint64_t memTotal = 0, memFree = 0, memAvailable = 0, buffers = 0, cached = 0;
    uint64_t swapTotal = 0, swapFree = 0;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key;
        uint64_t valKb = 0;
        ss >> key >> valKb;

        if (key == "MemTotal:") memTotal = valKb * 1024;
        else if (key == "MemFree:") memFree = valKb * 1024;
        else if (key == "MemAvailable:") memAvailable = valKb * 1024;
        else if (key == "Buffers:") buffers = valKb * 1024;
        else if (key == "Cached:") cached = valKb * 1024;
        else if (key == "SwapTotal:") swapTotal = valKb * 1024;
        else if (key == "SwapFree:") swapFree = valKb * 1024;
    }

    m_memory.totalRamBytes = memTotal;
    m_memory.freeRamBytes = memFree;
    m_memory.availableRamBytes = memAvailable;
    m_memory.buffersRamBytes = buffers;
    m_memory.cachedRamBytes = cached;
    m_memory.usedRamBytes = (memTotal > memAvailable) ? (memTotal - memAvailable) : (memTotal - memFree);

    m_memory.totalSwapBytes = swapTotal;
    m_memory.freeSwapBytes = swapFree;
    m_memory.usedSwapBytes = (swapTotal >= swapFree) ? (swapTotal - swapFree) : 0;

    if (memTotal > 0) {
        m_memory.ramUsagePercent = (100.0 * m_memory.usedRamBytes) / memTotal;
    }
    if (swapTotal > 0) {
        m_memory.swapUsagePercent = (100.0 * m_memory.usedSwapBytes) / swapTotal;
    }
}

void MetricsCollector::readNetwork() {
    std::ifstream file("/proc/net/dev");
    if (!file.is_open()) return;

    std::string line;
    // Skip header lines
    std::getline(file, line);
    std::getline(file, line);

    std::vector<NetworkInterfaceInfo> newInterfaces;
    uint64_t totalRx = 0;
    uint64_t totalTx = 0;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string ifaceName;
        ss >> ifaceName;
        if (ifaceName.back() == ':') ifaceName.pop_back();

        if (ifaceName == "lo") continue; // Skip loopback

        uint64_t rxBytes = 0, rxPackets = 0, rxErrs = 0, rxDrop = 0, rxFifo = 0, rxFrame = 0, rxCompressed = 0, rxMulticast = 0;
        uint64_t txBytes = 0;

        ss >> rxBytes >> rxPackets >> rxErrs >> rxDrop >> rxFifo >> rxFrame >> rxCompressed >> rxMulticast >> txBytes;

        totalRx += rxBytes;
        totalTx += txBytes;

        NetworkInterfaceInfo info;
        info.name = ifaceName;
        info.rxBytes = rxBytes;
        info.txBytes = txBytes;

        // Calculate rate based on previous values
        for (const auto& oldIface : m_network.interfaces) {
            if (oldIface.name == ifaceName) {
                if (rxBytes >= oldIface.rxBytes) {
                    info.rxRateBps = (rxBytes - oldIface.rxBytes) / m_lastSampleIntervalSec;
                }
                if (txBytes >= oldIface.txBytes) {
                    info.txRateBps = (txBytes - oldIface.txBytes) / m_lastSampleIntervalSec;
                }
                break;
            }
        }
        newInterfaces.push_back(info);
    }

    if (!m_firstRun && m_network.cumulativeRxBytes > 0) {
        if (totalRx >= m_network.cumulativeRxBytes) {
            m_network.totalRxRateBps = (totalRx - m_network.cumulativeRxBytes) / m_lastSampleIntervalSec;
        }
        if (totalTx >= m_network.cumulativeTxBytes) {
            m_network.totalTxRateBps = (totalTx - m_network.cumulativeTxBytes) / m_lastSampleIntervalSec;
        }
    }
    m_network.cumulativeRxBytes = totalRx;
    m_network.cumulativeTxBytes = totalTx;
    m_network.interfaces = newInterfaces;
}

void MetricsCollector::readDisk() {
    // 1. Read Disk I/O Rates from /proc/diskstats
    std::ifstream file("/proc/diskstats");
    if (file.is_open()) {
        std::string line;
        uint64_t totalReadSectors = 0;
        uint64_t totalWriteSectors = 0;

        while (std::getline(file, line)) {
            std::istringstream ss(line);
            int major = 0, minor = 0;
            std::string devName;
            ss >> major >> minor >> devName;

            if (devName.rfind("sd", 0) == 0 || devName.rfind("nvme", 0) == 0 || devName.rfind("vd", 0) == 0) {
                uint64_t readsCompleted = 0, readsMerged = 0, readSectors = 0, timeReading = 0;
                uint64_t writesCompleted = 0, writesMerged = 0, writeSectors = 0;
                ss >> readsCompleted >> readsMerged >> readSectors >> timeReading
                   >> writesCompleted >> writesMerged >> writeSectors;

                totalReadSectors += readSectors;
                totalWriteSectors += writeSectors;
            }
        }

        uint64_t readBytes = totalReadSectors * 512;
        uint64_t writeBytes = totalWriteSectors * 512;

        if (!m_firstRun && m_lastDiskReadBytes > 0) {
            if (readBytes >= m_lastDiskReadBytes) {
                m_disk.readRateBps = (readBytes - m_lastDiskReadBytes) / m_lastSampleIntervalSec;
            }
            if (writeBytes >= m_lastDiskWriteBytes) {
                m_disk.writeRateBps = (writeBytes - m_lastDiskWriteBytes) / m_lastSampleIntervalSec;
            }
        }
        m_lastDiskReadBytes = readBytes;
        m_lastDiskWriteBytes = writeBytes;
        m_disk.cumulativeReadBytes = readBytes;
        m_disk.cumulativeWriteBytes = writeBytes;
    }

    // 2. Read Mounted Filesystems from /proc/mounts
    std::ifstream mountsFile("/proc/mounts");
    if (mountsFile.is_open()) {
        std::string line;
        std::vector<DiskPartitionInfo> newPartitions;

        while (std::getline(mountsFile, line)) {
            std::istringstream ss(line);
            std::string dev, mountPoint, fsType;
            ss >> dev >> mountPoint >> fsType;

            // Only interested in real block devices (/dev/sd*, /dev/nvme*, /dev/mapper/*)
            if (dev.rfind("/dev/sd", 0) != 0 && dev.rfind("/dev/nvme", 0) != 0 && dev.rfind("/dev/mapper", 0) != 0) {
                continue;
            }

            // Skip swap mounts or pseudo mounts
            if (fsType == "swap" || fsType == "squashfs") continue;

            struct statvfs stat;
            if (statvfs(mountPoint.c_str(), &stat) == 0 && stat.f_blocks > 0) {
                DiskPartitionInfo part;
                part.device = dev;
                part.mountPoint = mountPoint;
                part.fsType = fsType;

                part.totalBytes = static_cast<uint64_t>(stat.f_blocks) * stat.f_frsize;
                part.freeBytes = static_cast<uint64_t>(stat.f_bfree) * stat.f_frsize;
                part.availableBytes = static_cast<uint64_t>(stat.f_bavail) * stat.f_frsize;
                part.usedBytes = (part.totalBytes > part.freeBytes) ? (part.totalBytes - part.freeBytes) : 0;

                if (part.totalBytes > 0) {
                    part.usagePercent = (100.0 * part.usedBytes) / part.totalBytes;
                }

                // Check if removable/USB (e.g., mounted under /run/media, /media, /mnt/usb, or removable flag)
                if (mountPoint.rfind("/run/media", 0) == 0 || mountPoint.rfind("/media", 0) == 0 ||
                    dev.find("usb") != std::string::npos) {
                    part.isUsbOrRemovable = true;
                }

                newPartitions.push_back(part);
            }
        }
        m_disk.partitions = newPartitions;
    }
}

void MetricsCollector::readProcesses() {
    std::vector<ProcessInfo> newProcList;
    std::unordered_map<int, std::pair<uint64_t, uint64_t>> newProcCpuTimes;

    long clockTicksPerSec = sysconf(_SC_CLK_TCK);
    if (clockTicksPerSec <= 0) clockTicksPerSec = 100;

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (!entry.is_directory()) continue;

        std::string filename = entry.path().filename().string();
        if (filename.empty() || !std::isdigit(filename[0])) continue;

        int pid = std::stoi(filename);

        // Read /proc/[pid]/stat
        std::ifstream statFile(entry.path() / "stat");
        if (!statFile.is_open()) continue;

        std::string line;
        if (!std::getline(statFile, line)) continue;

        // Process name is inside parentheses (e.g. "(bash)")
        size_t openParen = line.find('(');
        size_t closeParen = line.rfind(')');
        if (openParen == std::string::npos || closeParen == std::string::npos) continue;

        std::string comm = line.substr(openParen + 1, closeParen - openParen - 1);
        std::string rest = line.substr(closeParen + 2);

        std::istringstream ss(rest);
        char state = 'R';
        int ppid = 0, pgrp = 0, session = 0, tty_nr = 0, tpgid = 0;
        unsigned int flags = 0;
        uint64_t minflt = 0, cminflt = 0, majflt = 0, cmajflt = 0;
        uint64_t utime = 0, stime = 0;
        int64_t cutime = 0, cstime = 0, priority = 0, niceVal = 0;
        int64_t num_threads = 0, itrealvalue = 0;
        uint64_t starttime = 0, vsize = 0;
        int64_t rssPages = 0;

        ss >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags
           >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime
           >> cutime >> cstime >> priority >> niceVal >> num_threads >> itrealvalue
           >> starttime >> vsize >> rssPages;

        ProcessInfo pinfo;
        pinfo.pid = pid;
        pinfo.ppid = ppid;
        pinfo.name = comm;
        pinfo.state = state;
        pinfo.nice = static_cast<int>(niceVal);
        pinfo.utime = utime;
        pinfo.stime = stime;

        long pageSize = sysconf(_SC_PAGESIZE);
        if (pageSize <= 0) pageSize = 4096;
        pinfo.rssBytes = static_cast<uint64_t>(rssPages > 0 ? rssPages * pageSize : 0);

        // Read user owner
        struct stat st;
        if (stat(entry.path().c_str(), &st) == 0) {
            struct passwd *pw = getpwuid(st.st_uid);
            if (pw) pinfo.user = pw->pw_name;
            else pinfo.user = std::to_string(st.st_uid);
        }

        // Calculate CPU percentage
        uint64_t procTicks = utime + stime;
        if (m_lastProcCpuTimes.count(pid)) {
            uint64_t oldTicks = m_lastProcCpuTimes[pid].first;
            if (procTicks >= oldTicks) {
                uint64_t diffTicks = procTicks - oldTicks;
                double cpuSec = static_cast<double>(diffTicks) / clockTicksPerSec;
                pinfo.cpuPercent = (cpuSec / m_lastSampleIntervalSec) * 100.0;
            }
        }
        newProcCpuTimes[pid] = {procTicks, 0};

        newProcList.push_back(pinfo);
    }

    m_processes = std::move(newProcList);
    m_lastProcCpuTimes = std::move(newProcCpuTimes);
}

void MetricsCollector::scanDesktopEntries() {
    std::vector<std::string> dirs = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/share/applications"
    };

    for (const auto &dirPath : dirs) {
        if (dirPath.empty() || !fs::exists(dirPath)) continue;

        try {
            for (const auto &entry : fs::directory_iterator(dirPath)) {
                if (!entry.is_regular_file() || entry.path().extension() != ".desktop") continue;

                std::ifstream f(entry.path());
                if (!f.is_open()) continue;

                std::string line;
                std::string appName, iconName, execLine;
                bool isApp = false;
                bool noDisplay = false;

                while (std::getline(f, line)) {
                    if (line == "Type=Application") isApp = true;
                    if (line == "NoDisplay=true") noDisplay = true;
                    if (line.rfind("Name=", 0) == 0 && appName.empty()) appName = line.substr(5);
                    if (line.rfind("Icon=", 0) == 0 && iconName.empty()) iconName = line.substr(5);
                    if (line.rfind("Exec=", 0) == 0 && execLine.empty()) execLine = line.substr(5);
                }

                if (isApp && !noDisplay && !appName.empty() && !execLine.empty()) {
                    std::istringstream ss(execLine);
                    std::string execBinary;
                    ss >> execBinary;

                    size_t slashPos = execBinary.rfind('/');
                    if (slashPos != std::string::npos) {
                        execBinary = execBinary.substr(slashPos + 1);
                    }

                    if (!execBinary.empty()) {
                        m_desktopApps[execBinary] = { appName, iconName };
                        
                        if (execBinary == "brave-browser") m_desktopApps["brave"] = { appName, iconName };
                        if (execBinary == "telegram-desktop") m_desktopApps["telegram"] = { appName, iconName };
                        if (execBinary == "code-oss") m_desktopApps["code"] = { appName, iconName };
                        if (execBinary == "firefox-bin") m_desktopApps["firefox"] = { appName, iconName };
                    }
                }
            }
        } catch (...) {}
    }

    m_desktopApps["Taskmanager-Harbor"] = { "Taskmanager-Harbor", "taskmanager-harbor" };
    m_desktopApps["taskmanager-harbor"] = { "Taskmanager-Harbor", "taskmanager-harbor" };
}

void MetricsCollector::readApplicationGroups() {
    std::unordered_map<std::string, ApplicationGroup> appGroups;
    ApplicationGroup bgServicesGroup;
    bgServicesGroup.displayName = "Background Services";
    bgServicesGroup.iconName = "preferences-system";
    bgServicesGroup.user = "system";
    bgServicesGroup.isBackgroundService = true;

    for (const auto &p : m_processes) {
        if (p.name.empty() || p.name[0] == '[') continue;

        std::string rawName = p.name;
        
        if (m_desktopApps.count(rawName)) {
            const auto &meta = m_desktopApps[rawName];
            auto &grp = appGroups[meta.name];
            if (grp.displayName.empty()) {
                grp.displayName = meta.name;
                grp.iconName = meta.icon.empty() ? rawName : meta.icon;
                grp.user = p.user;
            }
            grp.pids.push_back(p.pid);
            grp.totalCpuPercent += p.cpuPercent;
            grp.totalRssBytes += p.rssBytes;
        } else {
            bgServicesGroup.pids.push_back(p.pid);
            bgServicesGroup.totalCpuPercent += p.cpuPercent;
            bgServicesGroup.totalRssBytes += p.rssBytes;
        }
    }

    std::vector<ApplicationGroup> resultList;
    for (auto &pair : appGroups) {
        resultList.push_back(pair.second);
    }

    std::sort(resultList.begin(), resultList.end(), [](const ApplicationGroup &a, const ApplicationGroup &b) {
        return a.totalRssBytes > b.totalRssBytes;
    });

    if (!bgServicesGroup.pids.empty()) {
        resultList.push_back(bgServicesGroup);
    }

    m_applications = std::move(resultList);
}

void MetricsCollector::readStaticSystemInfo() {
    struct utsname uts;
    if (uname(&uts) == 0) {
        m_sysInfo.osName = "Arch Linux";
        m_sysInfo.kernelVersion = std::string(uts.sysname) + " " + std::string(uts.release);
        m_sysInfo.architecture = uts.machine;
        m_sysInfo.hostname = uts.nodename;
    }

    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        int threads = 0;
        while (std::getline(cpuinfo, line)) {
            if (line.rfind("model name", 0) == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos && m_sysInfo.cpuModel.empty()) {
                    m_sysInfo.cpuModel = line.substr(colon + 2);
                }
            } else if (line.rfind("processor", 0) == 0) {
                threads++;
            }
        }
        m_sysInfo.cpuThreadCount = threads;
    }
}

} // namespace Harbor
