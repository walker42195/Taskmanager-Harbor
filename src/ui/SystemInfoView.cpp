#include "SystemInfoView.hpp"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <fstream>

namespace Harbor {

static QString formatUptime(uint64_t uptimeSec) {
    uint64_t days = uptimeSec / 86400;
    uint64_t hours = (uptimeSec % 86400) / 3600;
    uint64_t mins = (uptimeSec % 3600) / 60;
    uint64_t secs = uptimeSec % 60;

    QString res;
    if (days > 0) res += QString::number(days) + "d ";
    res += QString("%1h %2m %3s").arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
    return res;
}

SystemInfoView::SystemInfoView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(20);

    // Hero Badge Header
    auto *heroCard = new QWidget(this);
    heroCard->setStyleSheet("QWidget { background-color: #181c26; border: 1px solid #00d2ff; border-radius: 10px; padding: 12px; }");
    auto *heroLayout = new QVBoxLayout(heroCard);

    auto *titleLbl = new QLabel("⛵ Taskmanager-Harbor", heroCard);
    titleLbl->setStyleSheet("color: #00d2ff; font-size: 22px; font-weight: bold; border: none;");
    auto *subTitleLbl = new QLabel("High-Performance System Monitor for Linux", heroCard);
    subTitleLbl->setStyleSheet("color: #8b949e; font-size: 13px; border: none;");

    heroLayout->addWidget(titleLbl);
    heroLayout->addWidget(subTitleLbl);
    mainLayout->addWidget(heroCard);

    // Info Grid
    auto *gridWidget = new QWidget(this);
    auto *gridLayout = new QGridLayout(gridWidget);
    gridLayout->setSpacing(14);

    auto createCard = [&](const QString &title, QLabel *&valLabel, int row, int col) {
        auto *card = new QWidget();
        card->setStyleSheet("QWidget { background-color: #141722; border: 1px solid #232938; border-radius: 8px; }");
        auto *l = new QVBoxLayout(card);
        l->setContentsMargins(14, 12, 14, 12);
        l->setSpacing(4);

        auto *titleL = new QLabel(title, card);
        titleL->setStyleSheet("color: #7d8590; font-size: 11px; font-weight: bold; border: none;");

        valLabel = new QLabel("--", card);
        valLabel->setStyleSheet("color: #e0e6ed; font-size: 14px; font-weight: bold; border: none;");

        l->addWidget(titleL);
        l->addWidget(valLabel);
        gridLayout->addWidget(card, row, col);
    };

    createCard("OPERATIVSYSTEM", m_osLabel, 0, 0);
    createCard("KERNEL-VERSION", m_kernelLabel, 0, 1);
    createCard("HOSTNAMN", m_hostnameLabel, 1, 0);
    createCard("ARKITEKTUR", m_archLabel, 1, 1);
    createCard("PROCESSOR", m_cpuLabel, 2, 0);
    createCard("TRÅDAR / LOGISKA KÄRNOR", m_threadsLabel, 2, 1);
    createCard("SYSTEMMINNE", m_ramLabel, 3, 0);
    createCard("UPPTID (UPTIME)", m_uptimeLabel, 3, 1);

    mainLayout->addWidget(gridWidget);
    mainLayout->addStretch(1);
}

void SystemInfoView::updateSystemInfo(const SystemInfo &sysInfo) {
    m_osLabel->setText(QString::fromStdString(sysInfo.osName));
    m_kernelLabel->setText(QString::fromStdString(sysInfo.kernelVersion));
    m_hostnameLabel->setText(QString::fromStdString(sysInfo.hostname));
    m_archLabel->setText(QString::fromStdString(sysInfo.architecture));
    m_cpuLabel->setText(QString::fromStdString(sysInfo.cpuModel.empty() ? "x86_64 Processor" : sysInfo.cpuModel));
    m_threadsLabel->setText(QString::number(sysInfo.cpuThreadCount));

    // Read total uptime from /proc/uptime
    std::ifstream uptimeFile("/proc/uptime");
    if (uptimeFile.is_open()) {
        double uptimeSec = 0;
        if (uptimeFile >> uptimeSec) {
            updateUptime(static_cast<uint64_t>(uptimeSec));
        }
    }
}

void SystemInfoView::updateUptime(uint64_t uptimeSeconds) {
    m_uptimeLabel->setText(formatUptime(uptimeSeconds));
}

} // namespace Harbor
