#ifndef METRICSCOLLECTOR_HPP
#define METRICSCOLLECTOR_HPP

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <unordered_map>
#include "SystemMetrics.hpp"

namespace Harbor {

class MetricsCollector : public QObject {
    Q_OBJECT

public:
    explicit MetricsCollector(QObject *parent = nullptr);
    ~MetricsCollector() override = default;

    const CpuMetrics& cpuMetrics() const { return m_cpu; }
    const MemoryMetrics& memoryMetrics() const { return m_memory; }
    const NetworkMetrics& networkMetrics() const { return m_network; }
    const DiskMetrics& diskMetrics() const { return m_disk; }
    const std::vector<ProcessInfo>& processList() const { return m_processes; }
    const std::vector<ApplicationGroup>& applicationGroups() const { return m_applications; }
    const SystemInfo& systemInfo() const { return m_sysInfo; }

    void setUpdateInterval(int ms);

public slots:
    void updateAll();

signals:
    void metricsUpdated();

private:
    void readCpu();
    void readMemory();
    void readNetwork();
    void readDisk();
    void readProcesses();
    void readApplicationGroups();
    void scanDesktopEntries();
    void readStaticSystemInfo();

    struct DesktopAppMeta {
        std::string name;
        std::string icon;
    };

    QTimer m_timer;
    QElapsedTimer m_elapsed;
    double m_lastSampleIntervalSec{1.0};

    CpuMetrics m_cpu;
    MemoryMetrics m_memory;
    NetworkMetrics m_network;
    DiskMetrics m_disk;
    std::vector<ProcessInfo> m_processes;
    std::vector<ApplicationGroup> m_applications;
    std::unordered_map<std::string, DesktopAppMeta> m_desktopApps;
    SystemInfo m_sysInfo;

    // Previous state tracking for delta calculation
    std::unordered_map<int, std::pair<uint64_t, uint64_t>> m_lastProcCpuTimes; // pid -> (utime + stime, timestamp_ms)
    uint64_t m_lastTotalJiffies{0};
    uint64_t m_lastDiskReadBytes{0};
    uint64_t m_lastDiskWriteBytes{0};
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> m_lastPerDiskIo;
    bool m_firstRun{true};
};

} // namespace Harbor

#endif // METRICSCOLLECTOR_HPP
