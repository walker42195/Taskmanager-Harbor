#ifndef NETWORKVIEW_HPP
#define NETWORKVIEW_HPP

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include "GraphWidget.hpp"
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class NetworkView : public QWidget {
    Q_OBJECT

public:
    explicit NetworkView(QWidget *parent = nullptr);
    ~NetworkView() override = default;

    void updateNetwork(const NetworkMetrics &net);

private:
    GraphWidget *m_networkGraph;
    QLabel *m_totalRxLabel;
    QLabel *m_totalTxLabel;
    QLabel *m_cumulRxLabel;
    QLabel *m_cumulTxLabel;
    QTableWidget *m_ifaceTable;
};

} // namespace Harbor

#endif // NETWORKVIEW_HPP
