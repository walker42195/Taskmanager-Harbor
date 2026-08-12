#ifndef CPUVIEW_HPP
#define CPUVIEW_HPP

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QGridLayout>
#include <vector>
#include "GraphWidget.hpp"
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class CpuView : public QWidget {
    Q_OBJECT

public:
    explicit CpuView(QWidget *parent = nullptr);
    ~CpuView() override = default;

    void updateCpu(const CpuMetrics &cpu);

private:
    GraphWidget *m_totalCpuGraph;
    QLabel *m_modelLabel;
    QLabel *m_freqLabel;
    QLabel *m_threadsLabel;

    QWidget *m_coresContainer;
    QGridLayout *m_coresLayout;

    struct CoreUiComponents {
        QLabel *label;
        QProgressBar *bar;
        QLabel *pctLabel;
    };
    std::vector<CoreUiComponents> m_coreWidgets;
};

} // namespace Harbor

#endif // CPUVIEW_HPP
