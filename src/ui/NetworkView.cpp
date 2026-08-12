#include "NetworkView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

namespace Harbor {

static QString formatBytes(uint64_t bytes) {
    double b = static_cast<double>(bytes);
    if (b >= 1024.0 * 1024.0 * 1024.0) return QString::number(b / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    if (b >= 1024.0 * 1024.0) return QString::number(b / (1024.0 * 1024.0), 'f', 2) + " MB";
    if (b >= 1024.0) return QString::number(b / 1024.0, 'f', 2) + " KB";
    return QString::number(b, 'f', 0) + " B";
}

static QString formatRate(double bps) {
    if (bps >= 1024.0 * 1024.0 * 1024.0) return QString::number(bps / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB/s";
    if (bps >= 1024.0 * 1024.0) return QString::number(bps / (1024.0 * 1024.0), 'f', 2) + " MB/s";
    if (bps >= 1024.0) return QString::number(bps / 1024.0, 'f', 1) + " KB/s";
    return QString::number(bps, 'f', 0) + " B/s";
}

NetworkView::NetworkView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Summary Header Box
    auto *summaryBox = new QWidget(this);
    summaryBox->setStyleSheet("QWidget#summaryBox { background-color: #181c26; border: 1px solid #262c3a; border-radius: 8px; }");
    summaryBox->setObjectName("summaryBox");

    auto *summaryLayout = new QHBoxLayout(summaryBox);
    summaryLayout->setContentsMargins(16, 12, 16, 12);
    summaryLayout->setSpacing(20);

    m_totalRxLabel = new QLabel("⬇ In (RX): 0 B/s", this);
    m_totalRxLabel->setStyleSheet("color: #00d2ff; font-size: 14px; font-weight: bold;");

    m_totalTxLabel = new QLabel("⬆ Ut (TX): 0 B/s", this);
    m_totalTxLabel->setStyleSheet("color: #ff0080; font-size: 14px; font-weight: bold;");

    m_cumulRxLabel = new QLabel("Totalt In: 0 MB", this);
    m_cumulRxLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    m_cumulTxLabel = new QLabel("Totalt Ut: 0 MB", this);
    m_cumulTxLabel->setStyleSheet("color: #8b949e; font-size: 13px;");

    summaryLayout->addWidget(m_totalRxLabel);
    summaryLayout->addWidget(m_totalTxLabel);
    summaryLayout->addWidget(m_cumulRxLabel);
    summaryLayout->addWidget(m_cumulTxLabel);

    mainLayout->addWidget(summaryBox);

    // Dual Line Network Graph (Cyan = RX, Pink = TX)
    m_networkGraph = new GraphWidget(this);
    m_networkGraph->setTitle("Nätverkstrafik i realtid (Blå = In, Rosa = Ut)");
    m_networkGraph->setUnit("B/s");
    m_networkGraph->setColors(QColor(0, 210, 255), QColor(0, 210, 255, 30));
    m_networkGraph->setSecondaryColors(QColor(255, 0, 128), QColor(255, 0, 128, 25));
    m_networkGraph->setRange(0.0, 1024.0 * 1024.0, true); // Auto-scaling
    mainLayout->addWidget(m_networkGraph, 1);

    // Interface breakdown table
    m_ifaceTable = new QTableWidget(this);
    m_ifaceTable->setColumnCount(5);
    m_ifaceTable->setHorizontalHeaderLabels({"Gränssnitt", "Nedladdning (RX)", "Uppladdning (TX)", "Ackumulerat RX", "Ackumulerat TX"});
    m_ifaceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ifaceTable->verticalHeader()->setVisible(false);
    m_ifaceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ifaceTable->setStyleSheet(
        "QTableWidget { background-color: #12141c; color: #d0d7de; gridline-color: #1e2330; border: 1px solid #242936; border-radius: 8px; }"
        "QHeaderView::section { background-color: #191d29; color: #8b949e; border: none; border-bottom: 1px solid #282e3d; padding: 8px; font-weight: bold; }"
    );
    mainLayout->addWidget(m_ifaceTable, 1);
}

void NetworkView::updateNetwork(const NetworkMetrics &net) {
    m_networkGraph->addDualDataPoint(net.totalRxRateBps, net.totalTxRateBps);

    m_totalRxLabel->setText(QString("⬇ In (RX): %1").arg(formatRate(net.totalRxRateBps)));
    m_totalTxLabel->setText(QString("⬆ Ut (TX): %1").arg(formatRate(net.totalTxRateBps)));

    m_cumulRxLabel->setText(QString("Totalt Mottaget: %1").arg(formatBytes(net.cumulativeRxBytes)));
    m_cumulTxLabel->setText(QString("Totalt Skickat: %1").arg(formatBytes(net.cumulativeTxBytes)));

    m_ifaceTable->setRowCount(0);
    for (const auto &iface : net.interfaces) {
        int row = m_ifaceTable->rowCount();
        m_ifaceTable->insertRow(row);

        m_ifaceTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(iface.name)));
        
        auto *rxItem = new QTableWidgetItem(formatRate(iface.rxRateBps));
        rxItem->setForeground(QColor(0, 210, 255));
        m_ifaceTable->setItem(row, 1, rxItem);

        auto *txItem = new QTableWidgetItem(formatRate(iface.txRateBps));
        txItem->setForeground(QColor(255, 0, 128));
        m_ifaceTable->setItem(row, 2, txItem);

        m_ifaceTable->setItem(row, 3, new QTableWidgetItem(formatBytes(iface.rxBytes)));
        m_ifaceTable->setItem(row, 4, new QTableWidgetItem(formatBytes(iface.txBytes)));
    }
}

} // namespace Harbor
