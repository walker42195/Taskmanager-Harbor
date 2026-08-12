#ifndef MEMORYVIEW_HPP
#define MEMORYVIEW_HPP

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include "GraphWidget.hpp"
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class MemoryView : public QWidget {
    Q_OBJECT

public:
    explicit MemoryView(QWidget *parent = nullptr);
    ~MemoryView() override = default;

    void updateMemory(const MemoryMetrics &mem);

private:
    GraphWidget *m_ramGraph;
    GraphWidget *m_swapGraph;

    QLabel *m_totalRamLabel;
    QLabel *m_usedRamLabel;
    QLabel *m_cachedRamLabel;
    QLabel *m_swapTotalLabel;
    QLabel *m_swapUsedLabel;

    QProgressBar *m_ramProgressBar;
    QProgressBar *m_swapProgressBar;
};

} // namespace Harbor

#endif // MEMORYVIEW_HPP
