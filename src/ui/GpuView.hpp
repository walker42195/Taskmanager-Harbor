#ifndef GPUVIEW_HPP
#define GPUVIEW_HPP

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <vector>
#include "GraphWidget.hpp"
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class GpuView : public QWidget {
    Q_OBJECT

public:
    explicit GpuView(QWidget *parent = nullptr);
    ~GpuView() override = default;

    void updateGpu(const GpuMetrics &gpuMetrics);

private:
    GraphWidget *m_gpuGraph;
    QLabel *m_primaryModelLabel;
    QLabel *m_driverLabel;
    QLabel *m_vramTotalLabel;

    QWidget *m_gpuCardsContainer;
    QVBoxLayout *m_gpuCardsLayout;

    struct GpuCardWidgets {
        QLabel *nameLabel;
        QLabel *vendorBadge;
        QProgressBar *usageBar;
        QLabel *usagePctLabel;
        QProgressBar *vramBar;
        QLabel *vramTextLabel;
        QLabel *tempLabel;
        QLabel *powerLabel;
        QLabel *clockLabel;
    };
    std::vector<GpuCardWidgets> m_gpuWidgets;
};

} // namespace Harbor

#endif // GPUVIEW_HPP
