#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include "backend/MetricsCollector.hpp"
#include "ui/CpuView.hpp"
#include "ui/MemoryView.hpp"
#include "ui/NetworkView.hpp"
#include "ui/ProcessView.hpp"
#include "ui/SystemInfoView.hpp"

namespace Harbor {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onMetricsUpdated();

private:
    void applyDarkStyleSheet();

    MetricsCollector m_collector;

    QTabWidget *m_tabWidget;

    // Overview Tab components
    QWidget *m_overviewTab;
    GraphWidget *m_overviewCpuGraph;
    GraphWidget *m_overviewRamGraph;
    GraphWidget *m_overviewNetGraph;
    GraphWidget *m_overviewDiskGraph;

    // Sub views
    CpuView *m_cpuView;
    MemoryView *m_memoryView;
    NetworkView *m_networkView;
    ProcessView *m_processView;
    SystemInfoView *m_systemInfoView;

    // Top status badges
    QLabel *m_badgeCpu;
    QLabel *m_badgeRam;
    QLabel *m_badgeNet;
    QLabel *m_statusFooter;
};

} // namespace Harbor

#endif // MAINWINDOW_HPP
