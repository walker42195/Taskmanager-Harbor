#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QIcon>

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
    setWindowIcon(QIcon(":/assets/icons/logo.svg"));
    resize(1100, 750);
    setMinimumSize(500, 500);

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
    headerLayout->setSpacing(10);

    auto *logoIconLabel = new QLabel(headerWidget);
    logoIconLabel->setPixmap(QIcon(":/assets/icons/logo.svg").pixmap(26, 26));

    auto *titleLabel = new QLabel("Taskmanager-Harbor", headerWidget);
    titleLabel->setStyleSheet("font-size: 17px; font-weight: bold; color: #00d2ff;");

    m_badgeCpu = new QLabel("CPU: 0%", headerWidget);
    m_badgeCpu->setStyleSheet("background-color: #1e2433; color: #00d2ff; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 12px;");

    m_badgeRam = new QLabel("RAM: 0%", headerWidget);
    m_badgeRam->setStyleSheet("background-color: #1a2923; color: #00e676; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 12px;");

    m_badgeNet = new QLabel("NET: 0 B/s", headerWidget);
    m_badgeNet->setStyleSheet("background-color: #2b1f2e; color: #ff0080; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 12px;");

    headerLayout->addWidget(logoIconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch(1);
    headerLayout->addWidget(m_badgeCpu);
    headerLayout->addWidget(m_badgeRam);
    headerLayout->addWidget(m_badgeNet);

    mainLayout->addWidget(headerWidget);

    // --- Wrapping Tab Bar (FlowLayout up to 3 rows) ---
    m_tabBarWidget = new QWidget(this);
    m_flowLayout = new FlowLayout(m_tabBarWidget, 0, 6, 6);

    // Stacked Widget for Page Content
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setStyleSheet("QStackedWidget { background-color: #12141c; border: 1px solid #232838; border-radius: 8px; }");

    // --- Tab 1: Overview Grid ---
    m_overviewTab = new QWidget();
    auto *overviewLayout = new QGridLayout(m_overviewTab);
    overviewLayout->setContentsMargins(12, 12, 12, 12);
    overviewLayout->setSpacing(12);

    m_overviewCpuGraph = new GraphWidget(m_overviewTab);
    m_overviewCpuGraph->setTitle("CPU Usage (%)");
    m_overviewCpuGraph->setUnit("%");
    m_overviewCpuGraph->setColors(QColor(0, 210, 255), QColor(0, 210, 255, 40));

    m_overviewRamGraph = new GraphWidget(m_overviewTab);
    m_overviewRamGraph->setTitle("RAM Usage");
    m_overviewRamGraph->setUnit("%");
    m_overviewRamGraph->setColors(QColor(0, 230, 118), QColor(0, 230, 118, 40));

    m_overviewNetGraph = new GraphWidget(m_overviewTab);
    m_overviewNetGraph->setTitle("Network Traffic (In / Out)");
    m_overviewNetGraph->setUnit("B/s");
    m_overviewNetGraph->setColors(QColor(0, 210, 255), QColor(0, 210, 255, 30));
    m_overviewNetGraph->setSecondaryColors(QColor(255, 0, 128), QColor(255, 0, 128, 25));
    m_overviewNetGraph->setRange(0.0, 1024.0 * 1024.0, true);

    m_overviewDiskGraph = new GraphWidget(m_overviewTab);
    m_overviewDiskGraph->setTitle("Disk I/O Speed");
    m_overviewDiskGraph->setUnit("B/s");
    m_overviewDiskGraph->setColors(QColor(255, 183, 77), QColor(255, 183, 77, 35));
    m_overviewDiskGraph->setSecondaryColors(QColor(255, 112, 67), QColor(255, 112, 67, 25));
    m_overviewDiskGraph->setRange(0.0, 1024.0 * 1024.0, true);

    overviewLayout->addWidget(m_overviewCpuGraph, 0, 0);
    overviewLayout->addWidget(m_overviewRamGraph, 0, 1);
    overviewLayout->addWidget(m_overviewNetGraph, 1, 0);
    overviewLayout->addWidget(m_overviewDiskGraph, 1, 1);

    // --- Sub Views ---
    m_applicationsView = new ApplicationsView(this);
    m_cpuView = new CpuView(this);
    m_memoryView = new MemoryView(this);
    m_networkView = new NetworkView(this);
    m_diskView = new DiskView(this);
    m_processView = new ProcessView(this);
    m_systemInfoView = new SystemInfoView(this);

    // Add Tabs Helper
    auto addTabItem = [&](QWidget *pageWidget, const QIcon &icon, const QString &title) {
        int index = static_cast<int>(m_tabButtons.size());
        m_stackedWidget->addWidget(pageWidget);

        auto *btn = new QPushButton(icon, "  " + title, m_tabBarWidget);
        btn->setIconSize(QSize(18, 18));
        btn->setCursor(Qt::PointingHandCursor);

        connect(btn, &QPushButton::clicked, this, [this, index]() {
            onTabButtonClicked(index);
        });

        m_flowLayout->addWidget(btn);
        m_tabButtons.push_back(btn);
    };

    addTabItem(m_overviewTab, QIcon(":/assets/icons/overview.svg"), "Overview");
    addTabItem(m_applicationsView, QIcon(":/assets/icons/applications.svg"), "Applications");
    addTabItem(m_cpuView, QIcon(":/assets/icons/cpu.svg"), "CPU & Cores");
    addTabItem(m_memoryView, QIcon(":/assets/icons/memory.svg"), "Memory & Swap");
    addTabItem(m_networkView, QIcon(":/assets/icons/network.svg"), "Network");
    addTabItem(m_diskView, QIcon(":/assets/icons/disk.svg"), "Disks & Storage");
    addTabItem(m_processView, QIcon(":/assets/icons/processes.svg"), "Processes");
    addTabItem(m_systemInfoView, QIcon(":/assets/icons/system.svg"), "System Info");

    mainLayout->addWidget(m_tabBarWidget);
    mainLayout->addWidget(m_stackedWidget, 1);

    // Footer
    auto *footerWidget = new QWidget(this);
    auto *footerLayout = new QHBoxLayout(footerWidget);
    footerLayout->setContentsMargins(4, 2, 4, 2);

    m_statusFooter = new QLabel("System monitor active | Refreshing every second", footerWidget);
    m_statusFooter->setStyleSheet("color: #6e7681; font-size: 11px;");

    auto *copyrightLabel = new QLabel("© 2026 taskmanager-harbor · <a href=\"https://novabase.se\" style=\"color: #00d2ff; text-decoration: underline;\">novabase.se</a>", footerWidget);
    copyrightLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #8b949e;");
    copyrightLabel->setOpenExternalLinks(true);
    copyrightLabel->setCursor(Qt::PointingHandCursor);

    footerLayout->addWidget(m_statusFooter);
    footerLayout->addStretch(1);
    footerLayout->addWidget(copyrightLabel);

    mainLayout->addWidget(footerWidget);

    setCentralWidget(centralWidget);

    // Connect Collector & Actions
    connect(&m_collector, &MetricsCollector::metricsUpdated, this, &MainWindow::onMetricsUpdated);
    connect(m_applicationsView, &ApplicationsView::appActionTriggered, &m_collector, &MetricsCollector::updateAll);
    connect(m_processView, &ProcessView::processActionTriggered, &m_collector, &MetricsCollector::updateAll);

    // Select first tab
    onTabButtonClicked(0);

    // Initial system info update
    m_systemInfoView->updateSystemInfo(m_collector.systemInfo());
}

void MainWindow::onTabButtonClicked(int index) {
    if (index < 0 || index >= static_cast<int>(m_tabButtons.size())) return;

    m_stackedWidget->setCurrentIndex(index);
    updateTabButtonStyles(index);
    onMetricsUpdated();
}

void MainWindow::updateTabButtonStyles(int selectedIndex) {
    for (size_t i = 0; i < m_tabButtons.size(); ++i) {
        if (static_cast<int>(i) == selectedIndex) {
            m_tabButtons[i]->setStyleSheet(
                "QPushButton { background-color: #1a2233; color: #00d2ff; border: 1px solid #00d2ff; "
                "border-top: 2px solid #00d2ff; border-radius: 6px; padding: 8px 14px; font-weight: bold; font-size: 13px; text-align: left; }"
            );
        } else {
            m_tabButtons[i]->setStyleSheet(
                "QPushButton { background-color: #151824; color: #8b949e; border: 1px solid #232838; "
                "border-radius: 6px; padding: 8px 14px; font-weight: bold; font-size: 13px; text-align: left; }"
                "QPushButton:hover { background-color: #1e2436; color: #c9d1d9; border: 1px solid #323b52; }"
            );
        }
    }
}

void MainWindow::onMetricsUpdated() {
    const auto &cpu = m_collector.cpuMetrics();
    const auto &mem = m_collector.memoryMetrics();
    const auto &net = m_collector.networkMetrics();
    const auto &disk = m_collector.diskMetrics();

    // Always update Overview Tab Sparkline Data Buffers & Top Badges
    m_overviewCpuGraph->addDataPoint(cpu.totalUsagePercent);
    m_overviewRamGraph->addDataPoint(mem.ramUsagePercent);
    
    double totalGb = static_cast<double>(mem.totalRamBytes) / (1024.0 * 1024.0 * 1024.0);
    double usedGb = static_cast<double>(mem.usedRamBytes) / (1024.0 * 1024.0 * 1024.0);
    QString ramBadgeStr = QString("%1 GB / %2 GB (%3%)")
        .arg(QString::number(usedGb, 'f', 1))
        .arg(QString::number(totalGb, 'f', 1))
        .arg(QString::number(mem.ramUsagePercent, 'f', 1));
    m_overviewRamGraph->setCustomValueText(ramBadgeStr);

    m_overviewNetGraph->addDualDataPoint(net.totalRxRateBps, net.totalTxRateBps);
    m_overviewDiskGraph->addDualDataPoint(disk.readRateBps, disk.writeRateBps);

    // Top Badges
    m_badgeCpu->setText(QString("CPU: %1%").arg(QString::number(cpu.totalUsagePercent, 'f', 1)));
    m_badgeRam->setText(QString("RAM: %1 GB / %2 GB (%3%)").arg(QString::number(usedGb, 'f', 1)).arg(QString::number(totalGb, 'f', 1)).arg(QString::number(mem.ramUsagePercent, 'f', 1)));
    m_badgeNet->setText(QString("NET: ⬇ %1 / ⬆ %2").arg(formatRateShort(net.totalRxRateBps)).arg(formatRateShort(net.totalTxRateBps)));

    // LAZY TAB UPDATES: Only update the view that is currently selected!
    int currentTab = m_stackedWidget->currentIndex();
    switch (currentTab) {
        case 0: // Overview
            break;
        case 1: // Applications
            m_applicationsView->updateApplications(m_collector.applicationGroups());
            break;
        case 2: // CPU
            m_cpuView->updateCpu(cpu);
            break;
        case 3: // Memory
            m_memoryView->updateMemory(mem);
            break;
        case 4: // Network
            m_networkView->updateNetwork(net);
            break;
        case 5: // Disks
            m_diskView->updateDisk(disk);
            break;
        case 6: // Processes
            m_processView->updateProcesses(m_collector.processList());
            break;
        case 7: // System Info
            m_systemInfoView->updateSystemInfo(m_collector.systemInfo());
            break;
    }
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
