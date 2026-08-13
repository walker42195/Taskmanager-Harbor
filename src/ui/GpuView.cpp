#include "GpuView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>

namespace Harbor {

static QString formatGb(uint64_t bytes) {
    double gb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
    return QString::number(gb, 'f', 1) + " GB";
}

GpuView::GpuView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Header Card with Primary GPU Info
    auto *headerCard = new QWidget(this);
    headerCard->setStyleSheet("QWidget#headerCard { background-color: #181c26; border: 1px solid #262c3a; border-radius: 8px; }");
    headerCard->setObjectName("headerCard");

    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    m_primaryModelLabel = new QLabel("GPU: Detecting...", this);
    m_primaryModelLabel->setStyleSheet("color: #7c4dff; font-size: 15px; font-weight: bold;");

    m_driverLabel = new QLabel("Driver: --", this);
    m_driverLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    m_vramTotalLabel = new QLabel("Total VRAM: --", this);
    m_vramTotalLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    headerLayout->addWidget(m_primaryModelLabel, 2);
    headerLayout->addWidget(m_driverLabel);
    headerLayout->addWidget(m_vramTotalLabel);

    mainLayout->addWidget(headerCard);

    // Primary GPU & VRAM Sparkline Graph
    m_gpuGraph = new GraphWidget(this);
    m_gpuGraph->setTitle("GPU Core & VRAM Usage (%)");
    m_gpuGraph->setUnit("%");
    m_gpuGraph->setColors(QColor(124, 77, 255), QColor(124, 77, 255, 45)); // Purple Accent for GPU
    m_gpuGraph->setSecondaryColors(QColor(0, 230, 118), QColor(0, 230, 118, 30)); // Green for VRAM
    m_gpuGraph->setDualLabels("Core", "VRAM");
    m_gpuGraph->setRange(0.0, 100.0, false);
    mainLayout->addWidget(m_gpuGraph, 1);

    // Scroll Area for Detailed Per-GPU Cards
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; } QScrollBar { width: 8px; }");

    m_gpuCardsContainer = new QWidget();
    m_gpuCardsContainer->setStyleSheet("background: transparent;");
    m_gpuCardsLayout = new QVBoxLayout(m_gpuCardsContainer);
    m_gpuCardsLayout->setSpacing(12);

    scrollArea->setWidget(m_gpuCardsContainer);
    mainLayout->addWidget(scrollArea, 2);
}

void GpuView::updateGpu(const GpuMetrics &gpuMetrics) {
    m_gpuGraph->addDualDataPoint(gpuMetrics.primaryUsagePercent, gpuMetrics.totalVramUsagePercent);

    if (gpuMetrics.gpus.empty()) {
        m_primaryModelLabel->setText("GPU: Integrated / No GPU metrics detected");
        m_driverLabel->setText("Driver: N/A");
        m_vramTotalLabel->setText("Total VRAM: N/A");

        if (!m_gpuWidgets.empty()) {
            QLayoutItem *child;
            while ((child = m_gpuCardsLayout->takeAt(0)) != nullptr) {
                if (child->widget()) delete child->widget();
                delete child;
            }
            m_gpuWidgets.clear();
        }
        return;
    }

    const auto &primaryGpu = gpuMetrics.gpus[0];
    m_primaryModelLabel->setText(QString("GPU: %1").arg(QString::fromStdString(primaryGpu.name)));
    m_driverLabel->setText(QString("Driver: %1").arg(primaryGpu.driverVersion.empty() ? "Sysfs DRM" : QString::fromStdString(primaryGpu.driverVersion)));
    m_vramTotalLabel->setText(QString("Total VRAM: %1").arg(formatGb(gpuMetrics.totalVramTotalBytes)));

    size_t numGpus = gpuMetrics.gpus.size();

    // Rebuild GPU cards if count changed
    if (m_gpuWidgets.size() != numGpus) {
        // Clear layout
        QLayoutItem *child;
        while ((child = m_gpuCardsLayout->takeAt(0)) != nullptr) {
            if (child->widget()) delete child->widget();
            delete child;
        }
        m_gpuWidgets.clear();

        for (size_t i = 0; i < numGpus; ++i) {
            auto *card = new QWidget();
            card->setStyleSheet(
                "QWidget { background-color: #161922; border: 1px solid #232836; border-radius: 8px; padding: 12px; }"
            );

            auto *cardLayout = new QVBoxLayout(card);
            cardLayout->setSpacing(10);

            // Row 1: Name & Vendor Badge
            auto *row1 = new QHBoxLayout();
            auto *nameLbl = new QLabel(card);
            nameLbl->setStyleSheet("color: #ffffff; font-size: 14px; font-weight: bold; border: none;");

            auto *vendorBadge = new QLabel(card);
            vendorBadge->setStyleSheet("background-color: #281d45; color: #b388ff; padding: 2px 8px; border-radius: 10px; font-size: 11px; font-weight: bold; border: none;");

            row1->addWidget(nameLbl);
            row1->addWidget(vendorBadge);
            row1->addStretch(1);
            cardLayout->addLayout(row1);

            // Row 2: GPU Core Usage Bar
            auto *row2 = new QHBoxLayout();
            auto *usageTitle = new QLabel("Core Usage:", card);
            usageTitle->setStyleSheet("color: #8b949e; font-size: 12px; border: none;");
            usageTitle->setFixedWidth(120);

            auto *usageBar = new QProgressBar(card);
            usageBar->setRange(0, 100);
            usageBar->setTextVisible(false);
            usageBar->setFixedHeight(14);
            usageBar->setStyleSheet(
                "QProgressBar { background-color: #0d0f15; border: none; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: #7c4dff; border-radius: 4px; }"
            );

            auto *usagePctLbl = new QLabel("0%", card);
            usagePctLbl->setFixedWidth(50);
            usagePctLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            usagePctLbl->setStyleSheet("color: #7c4dff; font-size: 12px; font-weight: bold; border: none;");

            row2->addWidget(usageTitle);
            row2->addWidget(usageBar, 1);
            row2->addWidget(usagePctLbl);
            cardLayout->addLayout(row2);

            // Row 3: VRAM Usage Bar
            auto *row3 = new QHBoxLayout();
            auto *vramTitle = new QLabel("VRAM Usage:", card);
            vramTitle->setStyleSheet("color: #8b949e; font-size: 12px; border: none;");
            vramTitle->setFixedWidth(120);

            auto *vramBar = new QProgressBar(card);
            vramBar->setRange(0, 100);
            vramBar->setTextVisible(false);
            vramBar->setFixedHeight(14);
            vramBar->setStyleSheet(
                "QProgressBar { background-color: #0d0f15; border: none; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: #00e676; border-radius: 4px; }"
            );

            auto *vramTextLbl = new QLabel("0 GB / 0 GB", card);
            vramTextLbl->setMinimumWidth(160);
            vramTextLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            vramTextLbl->setStyleSheet("color: #00e676; font-size: 12px; font-weight: bold; border: none;");

            row3->addWidget(vramTitle);
            row3->addWidget(vramBar, 1);
            row3->addWidget(vramTextLbl);
            cardLayout->addLayout(row3);

            // Row 4: Details Badges (Temp, Power, Clock)
            auto *row4 = new QHBoxLayout();
            auto *tempLbl = new QLabel("Temp: -- °C", card);
            tempLbl->setStyleSheet("background-color: #1e2433; color: #ffab40; padding: 4px 10px; border-radius: 6px; font-size: 12px; border: none;");

            auto *powerLbl = new QLabel("Power: -- W", card);
            powerLbl->setStyleSheet("background-color: #1e2433; color: #448aff; padding: 4px 10px; border-radius: 6px; font-size: 12px; border: none;");

            auto *clockLbl = new QLabel("Clock: -- MHz", card);
            clockLbl->setStyleSheet("background-color: #1e2433; color: #e0e0e0; padding: 4px 10px; border-radius: 6px; font-size: 12px; border: none;");

            row4->addWidget(tempLbl);
            row4->addWidget(powerLbl);
            row4->addWidget(clockLbl);
            row4->addStretch(1);
            cardLayout->addLayout(row4);

            m_gpuCardsLayout->addWidget(card);
            m_gpuWidgets.push_back({nameLbl, vendorBadge, usageBar, usagePctLbl, vramBar, vramTextLbl, tempLbl, powerLbl, clockLbl});
        }
    }

    // Update GPU Card Values
    for (size_t i = 0; i < numGpus && i < m_gpuWidgets.size(); ++i) {
        const auto &gpu = gpuMetrics.gpus[i];
        auto &w = m_gpuWidgets[i];

        w.nameLabel->setText(QString::fromStdString(gpu.name));
        w.vendorBadge->setText(QString::fromStdString(gpu.vendor));

        w.usageBar->setValue(static_cast<int>(gpu.gpuUsagePercent));
        w.usagePctLabel->setText(QString::number(gpu.gpuUsagePercent, 'f', 0) + "%");

        w.vramBar->setValue(static_cast<int>(gpu.memUsagePercent));
        w.vramTextLabel->setText(QString("%1 / %2 (%3%)")
            .arg(formatGb(gpu.vramUsedBytes))
            .arg(formatGb(gpu.vramTotalBytes))
            .arg(QString::number(gpu.memUsagePercent, 'f', 0)));

        if (gpu.temperatureC > 0) {
            w.tempLabel->setText(QString("Temp: %1 °C").arg(QString::number(gpu.temperatureC, 'f', 0)));
            w.tempLabel->setVisible(true);
        } else {
            w.tempLabel->setVisible(false);
        }

        if (gpu.powerDrawW > 0) {
            w.powerLabel->setText(QString("Power: %1 W").arg(QString::number(gpu.powerDrawW, 'f', 1)));
            w.powerLabel->setVisible(true);
        } else {
            w.powerLabel->setVisible(false);
        }

        if (gpu.clockMHz > 0) {
            w.clockLabel->setText(QString("Clock: %1 MHz").arg(QString::number(gpu.clockMHz, 'f', 0)));
            w.clockLabel->setVisible(true);
        } else {
            w.clockLabel->setVisible(false);
        }
    }
}

} // namespace Harbor
