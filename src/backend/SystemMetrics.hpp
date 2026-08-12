#ifndef SYSTEM_METRICS_HPP
#define SYSTEM_METRICS_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace Harbor {

struct CoreTime {
    uint64_t user{0};
    uint64_t nice{0};
    uint64_t system{0};
    uint64_t idle{0};
    uint64_t iowait{0};
    uint64_t irq{0};
    uint64_t softirq{0};
    uint64_t steal{0};

    uint64_t total() const {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }
    uint64_t active() const {
        return user + nice + system + irq + softirq + steal;
    }
};

struct CpuMetrics {
    double totalUsagePercent{0.0};
    std::string modelName;
    double currentFreqMHz{0.0};
    std::vector<double> perCoreUsagePercent;
    std::vector<CoreTime> lastCoreTimes;
    CoreTime lastTotalTime;
};

struct MemoryMetrics {
    uint64_t totalRamBytes{0};
    uint64_t freeRamBytes{0};
    uint64_t availableRamBytes{0};
    uint64_t usedRamBytes{0};
    uint64_t buffersRamBytes{0};
    uint64_t cachedRamBytes{0};

    uint64_t totalSwapBytes{0};
    uint64_t freeSwapBytes{0};
    uint64_t usedSwapBytes{0};

    double ramUsagePercent{0.0};
    double swapUsagePercent{0.0};
};

struct NetworkInterfaceInfo {
    std::string name;
    uint64_t rxBytes{0};
    uint64_t txBytes{0};
    double rxRateBps{0.0}; // Bytes per sec
    double txRateBps{0.0};
};

struct NetworkMetrics {
    double totalRxRateBps{0.0};
    double totalTxRateBps{0.0};
    uint64_t cumulativeRxBytes{0};
    uint64_t cumulativeTxBytes{0};
    std::vector<NetworkInterfaceInfo> interfaces;
};

struct DiskMetrics {
    double readRateBps{0.0};
    double writeRateBps{0.0};
    uint64_t cumulativeReadBytes{0};
    uint64_t cumulativeWriteBytes{0};
};

struct ProcessInfo {
    int pid{0};
    int ppid{0};
    std::string name;
    std::string cmdline;
    std::string user;
    char state{'R'};
    double cpuPercent{0.0};
    uint64_t rssBytes{0}; // Resident Set Size (RAM)
    int nice{0};
    uint64_t utime{0};
    uint64_t stime{0};
};

struct SystemInfo {
    std::string hostname;
    std::string osName;
    std::string kernelVersion;
    std::string architecture;
    uint64_t uptimeSeconds{0};
    std::string cpuModel;
    int cpuThreadCount{0};
    uint64_t totalMemoryBytes{0};
};

} // namespace Harbor

#endif // SYSTEM_METRICS_HPP
