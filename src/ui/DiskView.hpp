#ifndef DISKVIEW_HPP
#define DISKVIEW_HPP

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QProgressBar>
#include <vector>
#include "GraphWidget.hpp"
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class DiskView : public QWidget {
    Q_OBJECT

public:
    explicit DiskView(QWidget *parent = nullptr);
    ~DiskView() override = default;

    void updateDisk(const DiskMetrics &disk);

private:
    GraphWidget *m_ioGraph;
    QLabel *m_readRateLabel;
    QLabel *m_writeRateLabel;
    QTableWidget *m_diskTable;
};

} // namespace Harbor

#endif // DISKVIEW_HPP
