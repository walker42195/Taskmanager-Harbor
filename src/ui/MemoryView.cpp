#include "MemoryView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

namespace Harbor {

MemoryView::MemoryView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Summary Info Header Box
    auto *summaryBox = new QWidget(this);
    summaryBox->setStyleSheet("QWidget#summaryBox { background-color: #181c26; border: 1px solid #262c3a; border-radius: 8px; }");
    summaryBox->setObjectName("summaryBox");

    auto *summaryLayout = new QHBoxLayout(summaryBox);
    summaryLayout->setContentsMargins(16, 12, 16, 12);
    summaryLayout->setSpacing(20);

    m_totalRamLabel = new QLabel("Total RAM: -- GB", this);
    m_totalRamLabel->setStyleSheet("color: #00e676; font-size: 14px; font-weight: bold;");

    m_usedRamLabel = new QLabel("Used: -- GB", this);
    m_usedRamLabel->setStyleSheet("color: #e0e6ed; font-size: 13px;");

    m_cachedRamLabel = new QLabel("Buffers/Cache: -- GB", this);
    m_cachedRamLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    m_swapTotalLabel = new QLabel("Total Swap: -- GB", this);
    m_swapTotalLabel->setStyleSheet("color: #ab47bc; font-size: 13px;");

    m_swapUsedLabel = new QLabel("Used Swap: -- GB", this);
    m_swapUsedLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    summaryLayout->addWidget(m_totalRamLabel);
    summaryLayout->addWidget(m_usedRamLabel);
    summaryLayout->addWidget(m_cachedRamLabel);
    summaryLayout->addWidget(m_swapTotalLabel);
    summaryLayout->addWidget(m_swapUsedLabel);

    mainLayout->addWidget(summaryBox);

    // RAM Sparkline Graph
    m_ramGraph = new GraphWidget(this);
    m_ramGraph->setTitle("RAM Memory Usage");
    m_ramGraph->setUnit("%");
    m_ramGraph->setColors(QColor(0, 230, 118), QColor(0, 230, 118, 45)); // Emerald Green
    m_ramGraph->setRange(0.0, 100.0, false);
    mainLayout->addWidget(m_ramGraph, 1);

    // Swap Sparkline Graph
    m_swapGraph = new GraphWidget(this);
    m_swapGraph->setTitle("Swap Memory Usage");
    m_swapGraph->setUnit("%");
    m_swapGraph->setColors(QColor(171, 71, 188), QColor(171, 71, 188, 45)); // Vibrant Purple
    m_swapGraph->setRange(0.0, 100.0, false);
    mainLayout->addWidget(m_swapGraph, 1);
}

void MemoryView::updateMemory(const MemoryMetrics &mem) {
    m_ramGraph->addDataPoint(mem.ramUsagePercent);
    m_swapGraph->addDataPoint(mem.swapUsagePercent);

    double totalGb = static_cast<double>(mem.totalRamBytes) / (1024.0 * 1024.0 * 1024.0);
    double usedGb = static_cast<double>(mem.usedRamBytes) / (1024.0 * 1024.0 * 1024.0);
    double cacheGb = static_cast<double>(mem.buffersRamBytes + mem.cachedRamBytes) / (1024.0 * 1024.0 * 1024.0);

    double swapTotalGb = static_cast<double>(mem.totalSwapBytes) / (1024.0 * 1024.0 * 1024.0);
    double swapUsedGb = static_cast<double>(mem.usedSwapBytes) / (1024.0 * 1024.0 * 1024.0);

    QString ramBadgeStr = QString("%1 GB / %2 GB (%3%)")
        .arg(QString::number(usedGb, 'f', 1))
        .arg(QString::number(totalGb, 'f', 1))
        .arg(QString::number(mem.ramUsagePercent, 'f', 1));
    m_ramGraph->setCustomValueText(ramBadgeStr);

    QString swapBadgeStr = QString("%1 GB / %2 GB (%3%)")
        .arg(QString::number(swapUsedGb, 'f', 1))
        .arg(QString::number(swapTotalGb, 'f', 1))
        .arg(QString::number(mem.swapUsagePercent, 'f', 1));
    m_swapGraph->setCustomValueText(swapBadgeStr);

    m_totalRamLabel->setText(QString("Total RAM: %1 GB").arg(QString::number(totalGb, 'f', 1)));
    m_usedRamLabel->setText(QString("Used: %1 GB (%2%)").arg(QString::number(usedGb, 'f', 1)).arg(QString::number(mem.ramUsagePercent, 'f', 1)));
    m_cachedRamLabel->setText(QString("Buffers/Cache: %1 GB").arg(QString::number(cacheGb, 'f', 1)));

    m_swapTotalLabel->setText(QString("Total Swap: %1 GB").arg(QString::number(swapTotalGb, 'f', 1)));
    m_swapUsedLabel->setText(QString("Used Swap: %1 GB (%2%)").arg(QString::number(swapUsedGb, 'f', 1)).arg(QString::number(mem.swapUsagePercent, 'f', 1)));
}

} // namespace Harbor
