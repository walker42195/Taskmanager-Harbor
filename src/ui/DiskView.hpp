#ifndef DISKVIEW_HPP
#define DISKVIEW_HPP

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QComboBox>
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

private slots:
    void onDiskSelectionChanged(int index);

private:
    GraphWidget *m_ioGraph;
    QComboBox *m_diskSelector;
    QLabel *m_readRateLabel;
    QLabel *m_writeRateLabel;
    QTableWidget *m_diskTable;

    DiskMetrics m_cachedDiskMetrics;
};

} // namespace Harbor

#endif // DISKVIEW_HPP
