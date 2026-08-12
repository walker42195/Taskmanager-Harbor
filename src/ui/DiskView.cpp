#include "DiskView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

namespace Harbor {

static QString formatBytes(uint64_t bytes) {
    double b = static_cast<double>(bytes);
    if (b >= 1024.0 * 1024.0 * 1024.0 * 1024.0) return QString::number(b / (1024.0 * 1024.0 * 1024.0 * 1024.0), 'f', 2) + " TB";
    if (b >= 1024.0 * 1024.0 * 1024.0) return QString::number(b / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
    if (b >= 1024.0 * 1024.0) return QString::number(b / (1024.0 * 1024.0), 'f', 1) + " MB";
    if (b >= 1024.0) return QString::number(b / 1024.0, 'f', 1) + " KB";
    return QString::number(b, 'f', 0) + " B";
}

static QString formatRate(double bps) {
    if (bps >= 1024.0 * 1024.0 * 1024.0) return QString::number(bps / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB/s";
    if (bps >= 1024.0 * 1024.0) return QString::number(bps / (1024.0 * 1024.0), 'f', 1) + " MB/s";
    if (bps >= 1024.0) return QString::number(bps / 1024.0, 'f', 1) + " KB/s";
    return QString::number(bps, 'f', 0) + " B/s";
}

DiskView::DiskView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Summary Header Box with Disk Selector Dropdown
    auto *summaryBox = new QWidget(this);
    summaryBox->setStyleSheet("QWidget#summaryBox { background-color: #181c26; border: 1px solid #262c3a; border-radius: 8px; }");
    summaryBox->setObjectName("summaryBox");

    auto *summaryLayout = new QHBoxLayout(summaryBox);
    summaryLayout->setContentsMargins(16, 12, 16, 12);
    summaryLayout->setSpacing(16);

    auto *selectorLabel = new QLabel("Select Disk:", this);
    selectorLabel->setStyleSheet("color: #8b949e; font-size: 13px; font-weight: bold;");

    m_diskSelector = new QComboBox(this);
    m_diskSelector->addItem("All Disks (Combined Total)", "all");
    m_diskSelector->setMinimumWidth(220);
    m_diskSelector->setStyleSheet(
        "QComboBox { background-color: #12151f; color: #00d2ff; border: 1px solid #2b3245; "
        "border-radius: 6px; padding: 5px 12px; font-weight: bold; font-size: 13px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background-color: #181c26; color: #e0e6ed; selection-background-color: #00d2ff; selection-color: #0d0f17; }"
    );

    m_readRateLabel = new QLabel("📖 Read: 0 B/s", this);
    m_readRateLabel->setStyleSheet("color: #ffb74d; font-size: 14px; font-weight: bold;");

    m_writeRateLabel = new QLabel("✍ Write: 0 B/s", this);
    m_writeRateLabel->setStyleSheet("color: #ff7043; font-size: 14px; font-weight: bold;");

    summaryLayout->addWidget(selectorLabel);
    summaryLayout->addWidget(m_diskSelector);
    summaryLayout->addSpacing(16);
    summaryLayout->addWidget(m_readRateLabel);
    summaryLayout->addWidget(m_writeRateLabel);
    summaryLayout->addStretch(1);

    mainLayout->addWidget(summaryBox);

    // Disk I/O Real-Time Graph
    m_ioGraph = new GraphWidget(this);
    m_ioGraph->setTitle("Real-Time Disk I/O Activity (Orange = Read, Coral = Write)");
    m_ioGraph->setUnit("B/s");
    m_ioGraph->setColors(QColor(255, 183, 77), QColor(255, 183, 77, 35));
    m_ioGraph->setSecondaryColors(QColor(255, 112, 67), QColor(255, 112, 67, 25));
    m_ioGraph->setRange(0.0, 1024.0 * 1024.0, true);
    mainLayout->addWidget(m_ioGraph, 1);

    // Mounted Disks Table
    m_diskTable = new QTableWidget(this);
    m_diskTable->setColumnCount(6);
    m_diskTable->setHorizontalHeaderLabels({"Drive / Type", "Mount Point", "Filesystem", "Used Space", "Free Space", "Total Capacity"});
    m_diskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_diskTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_diskTable->verticalHeader()->setVisible(false);
    m_diskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_diskTable->setStyleSheet(
        "QTableWidget { background-color: #12141c; color: #d0d7de; gridline-color: #1e2330; border: 1px solid #242936; border-radius: 8px; }"
        "QHeaderView::section { background-color: #191d29; color: #8b949e; border: none; border-bottom: 1px solid #282e3d; padding: 8px; font-weight: bold; }"
    );
    mainLayout->addWidget(m_diskTable, 2);

    connect(m_diskSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DiskView::onDiskSelectionChanged);
}

void DiskView::updateDisk(const DiskMetrics &disk) {
    m_cachedDiskMetrics = disk;

    // Update ComboBox devices if list changed
    for (const auto &dev : disk.devices) {
        QString devName = QString::fromStdString(dev.deviceName);
        if (m_diskSelector->findData(devName) == -1) {
            QString label = QString("%1 (Physical Disk)").arg(devName);
            if (devName.startsWith("nvme")) label = QString("%1 (NVMe SSD)").arg(devName);
            else if (devName.startsWith("sd")) label = QString("%1 (Drive / USB)").arg(devName);
            m_diskSelector->addItem(label, devName);
        }
    }

    // Determine current selection
    QString selectedDev = m_diskSelector->currentData().toString();
    double currentReadBps = disk.readRateBps;
    double currentWriteBps = disk.writeRateBps;

    if (selectedDev != "all" && !selectedDev.isEmpty()) {
        for (const auto &dev : disk.devices) {
            if (QString::fromStdString(dev.deviceName) == selectedDev) {
                currentReadBps = dev.readRateBps;
                currentWriteBps = dev.writeRateBps;
                break;
            }
        }
    }

    m_ioGraph->addDualDataPoint(currentReadBps, currentWriteBps);

    m_readRateLabel->setText(QString("📖 Read: %1").arg(formatRate(currentReadBps)));
    m_writeRateLabel->setText(QString("✍ Write: %1").arg(formatRate(currentWriteBps)));

    // Populate partition table
    m_diskTable->setRowCount(0);
    for (const auto &part : disk.partitions) {
        int row = m_diskTable->rowCount();
        m_diskTable->insertRow(row);

        QString typePrefix = "💾 Internal";
        if (part.isUsbOrRemovable) {
            typePrefix = "🔌 USB Drive";
        } else if (part.device.find("nvme") != std::string::npos) {
            typePrefix = "⚡ NVMe SSD";
        }

        m_diskTable->setItem(row, 0, new QTableWidgetItem(QString("%1 (%2)").arg(typePrefix, QString::fromStdString(part.device))));
        m_diskTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(part.mountPoint)));
        m_diskTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(part.fsType).toUpper()));

        QString usedStr = QString("%1 (%2%)").arg(formatBytes(part.usedBytes)).arg(QString::number(part.usagePercent, 'f', 1));
        auto *usedItem = new QTableWidgetItem(usedStr);
        if (part.usagePercent > 85.0) {
            usedItem->setForeground(QColor(255, 82, 82));
        } else if (part.usagePercent > 70.0) {
            usedItem->setForeground(QColor(255, 183, 77));
        } else {
            usedItem->setForeground(QColor(0, 230, 118));
        }
        m_diskTable->setItem(row, 3, usedItem);

        m_diskTable->setItem(row, 4, new QTableWidgetItem(formatBytes(part.availableBytes)));
        m_diskTable->setItem(row, 5, new QTableWidgetItem(formatBytes(part.totalBytes)));
    }
}

void DiskView::onDiskSelectionChanged(int index) {
    Q_UNUSED(index);
    updateDisk(m_cachedDiskMetrics);
}

} // namespace Harbor
