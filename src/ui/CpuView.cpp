#include "CpuView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>

namespace Harbor {

CpuView::CpuView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Header Card with Overall CPU Info
    auto *headerCard = new QWidget(this);
    headerCard->setStyleSheet("QWidget#headerCard { background-color: #181c26; border: 1px solid #262c3a; border-radius: 8px; }");
    headerCard->setObjectName("headerCard");

    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    m_modelLabel = new QLabel("Processor: Detecting...", this);
    m_modelLabel->setStyleSheet("color: #00d2ff; font-size: 15px; font-weight: bold;");

    m_freqLabel = new QLabel("Frequency: -- GHz", this);
    m_freqLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    m_threadsLabel = new QLabel("Threads: --", this);
    m_threadsLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    headerLayout->addWidget(m_modelLabel, 2);
    headerLayout->addWidget(m_freqLabel);
    headerLayout->addWidget(m_threadsLabel);

    mainLayout->addWidget(headerCard);

    // Total CPU Main Sparkline
    m_totalCpuGraph = new GraphWidget(this);
    m_totalCpuGraph->setTitle("Total CPU Usage (%)");
    m_totalCpuGraph->setUnit("%");
    m_totalCpuGraph->setColors(QColor(0, 210, 255), QColor(0, 210, 255, 45)); // Electric Cyan
    m_totalCpuGraph->setRange(0.0, 100.0, false);
    mainLayout->addWidget(m_totalCpuGraph, 1);

    // Scroll Area for Per-Core Breakdown Grid
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; } QScrollBar { width: 8px; }");

    m_coresContainer = new QWidget();
    m_coresContainer->setStyleSheet("background: transparent;");
    m_coresLayout = new QGridLayout(m_coresContainer);
    m_coresLayout->setSpacing(12);

    scrollArea->setWidget(m_coresContainer);
    mainLayout->addWidget(scrollArea, 2);
}

void CpuView::updateCpu(const CpuMetrics &cpu) {
    m_totalCpuGraph->addDataPoint(cpu.totalUsagePercent);

    if (!cpu.modelName.empty()) {
        m_modelLabel->setText(QString("Processor: %1").arg(QString::fromStdString(cpu.modelName)));
    } else {
        m_modelLabel->setText("Processor: x86_64 Processor");
    }

    if (cpu.currentFreqMHz > 0) {
        m_freqLabel->setText(QString("Frequency: %1 GHz").arg(QString::number(cpu.currentFreqMHz / 1000.0, 'f', 2)));
    }
    m_threadsLabel->setText(QString("Threads / Cores: %1").arg(cpu.perCoreUsagePercent.size()));

    size_t numCores = cpu.perCoreUsagePercent.size();

    // Dynamically build/expand grid layout if core count changed
    if (m_coreWidgets.size() != numCores) {
        // Clear existing
        for (auto &cw : m_coreWidgets) {
            delete cw.label;
            delete cw.bar;
            delete cw.pctLabel;
        }
        m_coreWidgets.clear();

        int columns = (numCores > 16) ? 4 : ((numCores > 8) ? 3 : 2);

        for (size_t i = 0; i < numCores; ++i) {
            auto *card = new QWidget();
            card->setStyleSheet(
                "QWidget { background-color: #161922; border: 1px solid #232836; border-radius: 6px; padding: 4px; }"
            );
            auto *cardLayout = new QHBoxLayout(card);
            cardLayout->setContentsMargins(8, 6, 8, 6);
            cardLayout->setSpacing(8);

            auto *lbl = new QLabel(QString("Core %1").arg(i), card);
            lbl->setStyleSheet("color: #a0aabe; font-size: 11px; font-weight: bold; border: none;");

            auto *bar = new QProgressBar(card);
            bar->setRange(0, 100);
            bar->setTextVisible(false);
            bar->setFixedHeight(12);
            bar->setStyleSheet(
                "QProgressBar { background-color: #0d0f15; border: none; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: #00d2ff; border-radius: 4px; }"
            );

            auto *pctLbl = new QLabel("0%", card);
            pctLbl->setFixedWidth(42);
            pctLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pctLbl->setStyleSheet("color: #00d2ff; font-size: 11px; font-weight: bold; border: none;");

            cardLayout->addWidget(lbl);
            cardLayout->addWidget(bar, 1);
            cardLayout->addWidget(pctLbl);

            int row = static_cast<int>(i) / columns;
            int col = static_cast<int>(i) % columns;
            m_coresLayout->addWidget(card, row, col);

            m_coreWidgets.push_back({lbl, bar, pctLbl});
        }
    }

    // Update core bars
    for (size_t i = 0; i < numCores && i < m_coreWidgets.size(); ++i) {
        double usage = cpu.perCoreUsagePercent[i];
        m_coreWidgets[i].bar->setValue(static_cast<int>(usage));
        m_coreWidgets[i].pctLabel->setText(QString::number(usage, 'f', 0) + "%");

        if (usage > 85.0) {
            m_coreWidgets[i].bar->setStyleSheet(
                "QProgressBar { background-color: #0d0f15; border: none; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: #ff5252; border-radius: 4px; }"
            );
            m_coreWidgets[i].pctLabel->setStyleSheet("color: #ff5252; font-size: 11px; font-weight: bold; border: none;");
        } else if (usage > 50.0) {
            m_coreWidgets[i].bar->setStyleSheet(
                "QProgressBar { background-color: #0d0f15; border: none; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: #ffb74d; border-radius: 4px; }"
            );
            m_coreWidgets[i].pctLabel->setStyleSheet("color: #ffb74d; font-size: 11px; font-weight: bold; border: none;");
        } else {
            m_coreWidgets[i].bar->setStyleSheet(
                "QProgressBar { background-color: #0d0f15; border: none; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: #00d2ff; border-radius: 4px; }"
            );
            m_coreWidgets[i].pctLabel->setStyleSheet("color: #00d2ff; font-size: 11px; font-weight: bold; border: none;");
        }
    }
}

} // namespace Harbor
