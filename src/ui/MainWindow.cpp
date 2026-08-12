#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QDateTime>

namespace Harbor {

static QString formatRateShort(double bps) {
    if (bps >= 1024.0 * 1024.0) return QString::number(bps / (1024.0 * 1024.0), 'f', 1) + " MB/s";
    if (bps >= 1024.0) return QString::number(bps / 1024.0, 'f', 0) + " KB/s";
    return QString::number(bps, 'f', 0) + " B/s";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Taskmanager-Harbor — System Monitor");
    resize(1100, 750);
    setMinimumSize(900, 600);

    applyDarkStyleSheet();

    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Top Header Bar
    auto *headerWidget = new QWidget(this);
    headerWidget->setObjectName("topHeader");
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(12, 8, 12, 8);

    auto *titleLabel = new QLabel("⛵ Taskmanager-Harbor", headerWidget);
    titleLabel->setStyleSheet("font-size: 17px; font-weight: bold; color: #00d2ff;");

    m_badgeCpu = new QLabel("CPU: 0%", headerWidget);
    m_badgeCpu->setStyleSheet("background-color: #1e2433; color: #00d2ff; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 12px;");

    m_badgeRam = new QLabel("RAM: 0%", headerWidget);
    m_badgeRam->setStyleSheet("background-color: #1a2923; color: #00e676; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 12px;");

    m_badgeNet = new QLabel("NET: 0 B/s", headerWidget);
    m_badgeNet->setStyleSheet("background-color: #2b1f2e; color: #ff0080; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 12px;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch(1);
    headerLayout->addWidget(m_badgeCpu);
    headerLayout->addWidget(m_badgeRam);
    headerLayout->addWidget(m_badgeNet);

    mainLayout->addWidget(headerWidget);

    // Main Tab Widget
    m_tabWidget = new QTabWidget(this);

    // --- Tab 1: Overview Grid ---
    m_overviewTab = new QWidget();
    auto *overviewLayout = new QGridLayout(m_overviewTab);
    overviewLayout->setContentsMargins(12, 12, 12, 12);
    overviewLayout->setSpacing(12);

    m_overviewCpuGraph = new GraphWidget(m_overviewTab);
    m_overviewCpuGraph->setTitle("CPU-belastning (%)");
    m_overviewCpuGraph->setUnit("%");
    m_overviewCpuGraph->setColors(QColor(0, 210, 255), QColor(0, 210, 255, 40));

    m_overviewRamGraph = new GraphWidget(m_overviewTab);
    m_overviewRamGraph->setTitle("RAM-användning (%)");
    m_overviewRamGraph->setUnit("%");
    m_overviewRamGraph->setColors(QColor(0, 230, 118), QColor(0, 230, 118, 40));

    m_overviewNetGraph = new GraphWidget(m_overviewTab);
    m_overviewNetGraph->setTitle("Nätverkstrafik (In / Ut)");
    m_overviewNetGraph->setUnit("B/s");
    m_overviewNetGraph->setColors(QColor(0, 210, 255), QColor(0, 210, 255, 30));
    m_overviewNetGraph->setSecondaryColors(QColor(255, 0, 128), QColor(255, 0, 128, 25));
    m_overviewNetGraph->setRange(0.0, 1024.0 * 1024.0, true);

    m_overviewDiskGraph = new GraphWidget(m_overviewTab);
    m_overviewDiskGraph->setTitle("Disk I/O Hastighet");
    m_overviewDiskGraph->setUnit("B/s");
    m_overviewDiskGraph->setColors(QColor(255, 183, 77), QColor(255, 183, 77, 35));
    m_overviewDiskGraph->setSecondaryColors(QColor(255, 112, 67), QColor(255, 112, 67, 25));
    m_overviewDiskGraph->setRange(0.0, 1024.0 * 1024.0, true);

    overviewLayout->addWidget(m_overviewCpuGraph, 0, 0);
    overviewLayout->addWidget(m_overviewRamGraph, 0, 1);
    overviewLayout->addWidget(m_overviewNetGraph, 1, 0);
    overviewLayout->addWidget(m_overviewDiskGraph, 1, 1);

    // --- Tab 2: CPU View ---
    m_cpuView = new CpuView(this);

    // --- Tab 3: Memory View ---
    m_memoryView = new MemoryView(this);

    // --- Tab 4: Network View ---
    m_networkView = new NetworkView(this);

    // --- Tab 5: Process View ---
    m_processView = new ProcessView(this);

    // --- Tab 6: System Info ---
    m_systemInfoView = new SystemInfoView(this);

    m_tabWidget->addTab(m_overviewTab, "📊 Översikt");
    m_tabWidget->addTab(m_cpuView, "💻 CPU & Kärnor");
    m_tabWidget->addTab(m_memoryView, "🧠 Minne & Swap");
    m_tabWidget->addTab(m_networkView, "🌐 Nätverk");
    m_tabWidget->addTab(m_processView, "⚡ Processer");
    m_tabWidget->addTab(m_systemInfoView, "ℹ️ Systeminfo");

    mainLayout->addWidget(m_tabWidget, 1);

    // Footer
    m_statusFooter = new QLabel("Systemövervakning igång | Uppdateras varje sekund", this);
    m_statusFooter->setStyleSheet("color: #6e7681; font-size: 11px; margin-top: 4px;");
    mainLayout->addWidget(m_statusFooter);

    setCentralWidget(centralWidget);

    // Connect Collector
    connect(&m_collector, &MetricsCollector::metricsUpdated, this, &MainWindow::onMetricsUpdated);
    connect(m_processView, &ProcessView::processActionTriggered, &m_collector, &MetricsCollector::updateAll);

    // Initial system info update
    m_systemInfoView->updateSystemInfo(m_collector.systemInfo());
}

void MainWindow::onMetricsUpdated() {
    const auto &cpu = m_collector.cpuMetrics();
    const auto &mem = m_collector.memoryMetrics();
    const auto &net = m_collector.networkMetrics();
    const auto &disk = m_collector.diskMetrics();
    const auto &procs = m_collector.processList();

    // Update Overview Tab Graphs
    m_overviewCpuGraph->addDataPoint(cpu.totalUsagePercent);
    m_overviewRamGraph->addDataPoint(mem.ramUsagePercent);
    m_overviewNetGraph->addDualDataPoint(net.totalRxRateBps, net.totalTxRateBps);
    m_overviewDiskGraph->addDualDataPoint(disk.readRateBps, disk.writeRateBps);

    // Update Detail Views
    m_cpuView->updateCpu(cpu);
    m_memoryView->updateMemory(mem);
    m_networkView->updateNetwork(net);
    m_processView->updateProcesses(procs);

    // Top Badges
    m_badgeCpu->setText(QString("CPU: %1%").arg(QString::number(cpu.totalUsagePercent, 'f', 1)));
    m_badgeRam->setText(QString("RAM: %1%").arg(QString::number(mem.ramUsagePercent, 'f', 1)));
    m_badgeNet->setText(QString("NET: ⬇ %1 / ⬆ %2").arg(formatRateShort(net.totalRxRateBps)).arg(formatRateShort(net.totalTxRateBps)));
}

void MainWindow::applyDarkStyleSheet() {
    QString qss = R"(
        QMainWindow {
            background-color: #0d0f17;
        }
        QWidget {
            color: #d0d7de;
            font-family: 'Cantarell', 'Segoe UI', 'Ubuntu', sans-serif;
        }
        QWidget#topHeader {
            background-color: #141722;
            border-bottom: 1px solid #232838;
            border-radius: 6px;
        }
        QTabWidget::pane {
            border: 1px solid #232838;
            background-color: #12141c;
            border-radius: 8px;
            top: -1px;
        }
        QTabBar::tab {
            background-color: #171b26;
            color: #8b949e;
            padding: 10px 18px;
            margin-right: 4px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            font-weight: bold;
            font-size: 13px;
        }
        QTabBar::tab:hover {
            background-color: #202636;
            color: #c9d1d9;
        }
        QTabBar::tab:selected {
            background-color: #12141c;
            color: #00d2ff;
            border-top: 2px solid #00d2ff;
        }
        QScrollBar:vertical {
            border: none;
            background: #12141c;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #2b3245;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: #00d2ff;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
    qApp->setStyleSheet(qss);
}

} // namespace Harbor
