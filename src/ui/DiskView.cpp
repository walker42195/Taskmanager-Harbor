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

    // Summary Header Box
    auto *summaryBox = new QWidget(this);
    summaryBox->setStyleSheet("QWidget#summaryBox { background-color: #181c26; border: 1px solid #262c3a; border-radius: 8px; }");
    summaryBox->setObjectName("summaryBox");

    auto *summaryLayout = new QHBoxLayout(summaryBox);
    summaryLayout->setContentsMargins(16, 12, 16, 12);
    summaryLayout->setSpacing(20);

    m_readRateLabel = new QLabel("📖 Disk Read: 0 B/s", this);
    m_readRateLabel->setStyleSheet("color: #ffb74d; font-size: 14px; font-weight: bold;");

    m_writeRateLabel = new QLabel("✍ Disk Write: 0 B/s", this);
    m_writeRateLabel->setStyleSheet("color: #ff7043; font-size: 14px; font-weight: bold;");

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
}

void DiskView::updateDisk(const DiskMetrics &disk) {
    m_ioGraph->addDualDataPoint(disk.readRateBps, disk.writeRateBps);

    m_readRateLabel->setText(QString("📖 Disk Read: %1").arg(formatRate(disk.readRateBps)));
    m_writeRateLabel->setText(QString("✍ Disk Write: %1").arg(formatRate(disk.writeRateBps)));

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

        // Used space with percentage
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

} // namespace Harbor
