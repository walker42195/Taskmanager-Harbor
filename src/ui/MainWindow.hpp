#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include "backend/MetricsCollector.hpp"
#include "ui/FlowLayout.hpp"
#include "ui/CpuView.hpp"
#include "ui/ApplicationsView.hpp"
#include "ui/MemoryView.hpp"
#include "ui/NetworkView.hpp"
#include "ui/DiskView.hpp"
#include "ui/ProcessView.hpp"
#include "ui/SystemInfoView.hpp"
#include "ui/GpuView.hpp"

namespace Harbor {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onTabButtonClicked(int index);
    void onMetricsUpdated();

private:
    void applyDarkStyleSheet();
    void updateTabButtonStyles(int selectedIndex);

    MetricsCollector m_collector;

    QWidget *m_tabBarWidget;
    FlowLayout *m_flowLayout;
    std::vector<QPushButton*> m_tabButtons;

    QStackedWidget *m_stackedWidget;

    // Overview Tab components
    QWidget *m_overviewTab;
    GraphWidget *m_overviewCpuGraph;
    GraphWidget *m_overviewGpuGraph;
    GraphWidget *m_overviewRamGraph;
    GraphWidget *m_overviewNetGraph;
    GraphWidget *m_overviewDiskGraph;

    // Sub views
    ApplicationsView *m_applicationsView;
    CpuView *m_cpuView;
    GpuView *m_gpuView;
    MemoryView *m_memoryView;
    NetworkView *m_networkView;
    DiskView *m_diskView;
    ProcessView *m_processView;
    SystemInfoView *m_systemInfoView;

    // Top status badges
    QLabel *m_badgeCpu;
    QLabel *m_badgeGpu;
    QLabel *m_badgeRam;
    QLabel *m_badgeNet;
    QLabel *m_statusFooter;
};

} // namespace Harbor

#endif // MAINWINDOW_HPP
