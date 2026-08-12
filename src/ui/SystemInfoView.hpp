#ifndef SYSTEMINFOVIEW_HPP
#define SYSTEMINFOVIEW_HPP

#include <QWidget>
#include <QLabel>
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class SystemInfoView : public QWidget {
    Q_OBJECT

public:
    explicit SystemInfoView(QWidget *parent = nullptr);
    ~SystemInfoView() override = default;

    void updateSystemInfo(const SystemInfo &sysInfo);
    void updateUptime(uint64_t uptimeSeconds);

private:
    QLabel *m_osLabel;
    QLabel *m_kernelLabel;
    QLabel *m_hostnameLabel;
    QLabel *m_archLabel;
    QLabel *m_cpuLabel;
    QLabel *m_threadsLabel;
    QLabel *m_ramLabel;
    QLabel *m_uptimeLabel;
};

} // namespace Harbor

#endif // SYSTEMINFOVIEW_HPP
