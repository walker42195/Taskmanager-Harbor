#include "ProcessView.hpp"
#include "backend/ProcessManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <algorithm>

namespace Harbor {

class NumericTableWidgetItem : public QTableWidgetItem {
public:
    enum Type { Double, Int, Bytes };
    NumericTableWidgetItem(double val, Type t, const QString &displayStr)
        : QTableWidgetItem(displayStr), m_numValue(val), m_type(t) {}

    bool operator<(const QTableWidgetItem &other) const override {
        const auto *o = dynamic_cast<const NumericTableWidgetItem*>(&other);
        if (o) return m_numValue < o->m_numValue;
        return QTableWidgetItem::operator<(other);
    }
private:
    double m_numValue{0.0};
    Type m_type{Double};
};

ProcessView::ProcessView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // Top Action & Search Bar
    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍 Search process name or PID...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color: #181c26; color: #e0e6ed; border: 1px solid #2b3245; "
        "border-radius: 6px; padding: 6px 12px; font-size: 13px; }"
        "QLineEdit:focus { border: 1px solid #00d2ff; }"
    );

    m_terminateBtn = new QPushButton("End Task (SIGTERM)", this);
    m_terminateBtn->setStyleSheet(
        "QPushButton { background-color: #2b3245; color: #ffb74d; border: 1px solid #3e4860; "
        "border-radius: 6px; padding: 6px 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #3b455e; color: #ffa726; }"
    );

#ifdef _WIN32
    m_killBtn = new QPushButton("End Task", this);
#else
    m_killBtn = new QPushButton("End Task / Force Kill", this);
#endif
    m_killBtn->setStyleSheet(
        "QPushButton { background-color: #3e1b24; color: #ff5252; border: 1px solid #5c2330; "
        "border-radius: 6px; padding: 6px 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #58212f; color: #ff1744; }"
    );

    topLayout->addWidget(m_searchEdit, 2);
    topLayout->addWidget(m_terminateBtn);
    topLayout->addWidget(m_killBtn);

    mainLayout->addLayout(topLayout);

    // Process Table Widget
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(7);
    m_tableWidget->setHorizontalHeaderLabels({"PID", "Name", "User", "CPU %", "Memory (RAM)", "Status", "Nice"});
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setSortingEnabled(true);
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);

    m_tableWidget->setStyleSheet(
        "QTableWidget { background-color: #12141c; color: #d0d7de; gridline-color: #1e2330; border: 1px solid #242936; border-radius: 8px; }"
        "QHeaderView::section { background-color: #191d29; color: #8b949e; border: none; border-bottom: 1px solid #282e3d; padding: 8px; font-weight: bold; }"
        "QTableWidget::item:selected { background-color: #1f364d; color: #00d2ff; }"
    );

    mainLayout->addWidget(m_tableWidget);

    // Status Footer
    m_statusLabel = new QLabel("Total: 0 processes", this);
    m_statusLabel->setStyleSheet("color: #7d8590; font-size: 12px;");
    mainLayout->addWidget(m_statusLabel);

    // Connections
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ProcessView::onSearchTextChanged);
    connect(m_terminateBtn, &QPushButton::clicked, this, &ProcessView::onTerminateSelected);
    connect(m_killBtn, &QPushButton::clicked, this, &ProcessView::onKillSelected);
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested, this, &ProcessView::showContextMenu);
}

void ProcessView::updateProcesses(const std::vector<ProcessInfo> &processes) {
    m_cachedProcesses = processes;

    int selectedPid = getSelectedPid();

    m_tableWidget->setUpdatesEnabled(false);
    m_tableWidget->setSortingEnabled(false);
    m_tableWidget->setRowCount(0);

    int displayCount = 0;
    int targetRowToSelect = -1;

    for (const auto &p : processes) {
        if (!m_filterText.isEmpty()) {
            QString pidStr = QString::number(p.pid);
            QString nameStr = QString::fromStdString(p.name);
            if (!pidStr.contains(m_filterText, Qt::CaseInsensitive) &&
                !nameStr.contains(m_filterText, Qt::CaseInsensitive)) {
                continue;
            }
        }

        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        if (p.pid == selectedPid) {
            targetRowToSelect = row;
        }

        // PID
        m_tableWidget->setItem(row, 0, new NumericTableWidgetItem(p.pid, NumericTableWidgetItem::Int, QString::number(p.pid)));
        
        // Name
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(p.name)));
        
        // User
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(p.user)));

        // CPU %
        QString cpuStr = QString::number(p.cpuPercent, 'f', 1) + "%";
        auto *cpuItem = new NumericTableWidgetItem(p.cpuPercent, NumericTableWidgetItem::Double, cpuStr);
        if (p.cpuPercent > 50.0) cpuItem->setForeground(QColor(255, 82, 82));
        else if (p.cpuPercent > 10.0) cpuItem->setForeground(QColor(255, 183, 77));
        m_tableWidget->setItem(row, 3, cpuItem);

        // Memory (RSS)
        double memMb = static_cast<double>(p.rssBytes) / (1024.0 * 1024.0);
        QString memStr = QString::number(memMb, 'f', 1) + " MB";
        auto *memItem = new NumericTableWidgetItem(memMb, NumericTableWidgetItem::Double, memStr);
        m_tableWidget->setItem(row, 4, memItem);

        // State
        QString stateStr;
        switch (p.state) {
            case 'R': stateStr = "Running (R)"; break;
            case 'S': stateStr = "Sleeping (S)"; break;
            case 'D': stateStr = "Disk Sleep (D)"; break;
            case 'Z': stateStr = "Zombie (Z)"; break;
            case 'T': stateStr = "Stopped (T)"; break;
            default: stateStr = QString(p.state); break;
        }
        m_tableWidget->setItem(row, 5, new QTableWidgetItem(stateStr));

        // Nice
        m_tableWidget->setItem(row, 6, new NumericTableWidgetItem(p.nice, NumericTableWidgetItem::Int, QString::number(p.nice)));

        displayCount++;
    }

    m_tableWidget->setSortingEnabled(true);

    if (targetRowToSelect >= 0 && targetRowToSelect < m_tableWidget->rowCount()) {
        m_tableWidget->selectRow(targetRowToSelect);
    }

    m_tableWidget->setUpdatesEnabled(true);
    m_statusLabel->setText(QString("Showing %1 of %2 processes").arg(displayCount).arg(processes.size()));
}

void ProcessView::onSearchTextChanged(const QString &text) {
    m_filterText = text.trimmed();
    updateProcesses(m_cachedProcesses);
}

int ProcessView::getSelectedPid() const {
    int row = m_tableWidget->currentRow();
    if (row < 0) return -1;
    auto *item = m_tableWidget->item(row, 0);
    if (!item) return -1;
    return item->text().toInt();
}

QString ProcessView::getSelectedProcessName() const {
    int row = m_tableWidget->currentRow();
    if (row < 0) return "";
    auto *item = m_tableWidget->item(row, 1);
    if (!item) return "";
    return item->text();
}

void ProcessView::onTerminateSelected() {
    int pid = getSelectedPid();
    if (pid <= 0) return;
    QString name = getSelectedProcessName();

    if (ProcessManager::terminateProcess(pid)) {
        m_statusLabel->setText(QString("Sent SIGTERM to %1 (PID %2)").arg(name).arg(pid));
        emit processActionTriggered();
    } else {
        QMessageBox::warning(this, "Error", QString("Could not terminate process %1 (PID %2). Permission denied.").arg(name).arg(pid));
    }
}

void ProcessView::onKillSelected() {
    int pid = getSelectedPid();
    if (pid <= 0) return;
    QString name = getSelectedProcessName();

    auto reply = QMessageBox::question(this, "Confirm End Process",
        QString("Are you sure you want to end process (%1) PID %2?").arg(name).arg(pid),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (ProcessManager::killProcess(pid)) {
            m_statusLabel->setText(QString("Ended process %1 (PID %2)").arg(name).arg(pid));
            emit processActionTriggered();
        } else {
            QMessageBox::warning(this, "Error", QString("Could not end process %1 (PID %2).").arg(name).arg(pid));
        }
    }
}

void ProcessView::onPauseSelected() {
    int pid = getSelectedPid();
    if (pid <= 0) return;
    if (ProcessManager::pauseProcess(pid)) {
        m_statusLabel->setText(QString("Paused PID %1").arg(pid));
    }
}

void ProcessView::onResumeSelected() {
    int pid = getSelectedPid();
    if (pid <= 0) return;
    if (ProcessManager::resumeProcess(pid)) {
        m_statusLabel->setText(QString("Resumed PID %1").arg(pid));
    }
}

void ProcessView::onChangeNiceSelected() {
    int pid = getSelectedPid();
    if (pid <= 0) return;

    bool ok = false;
    int curNice = 0;
    int row = m_tableWidget->currentRow();
    if (row >= 0 && m_tableWidget->item(row, 6)) {
        curNice = m_tableWidget->item(row, 6)->text().toInt();
    }

    int newNice = QInputDialog::getInt(this, "Change Nice Value (Priority)",
        QString("Enter new nice value for PID %1 (-20 = Highest Priority, 19 = Lowest Priority):").arg(pid),
        curNice, -20, 19, 1, &ok);

    if (ok) {
        if (ProcessManager::setPriority(pid, newNice)) {
            m_statusLabel->setText(QString("Changed nice for PID %1 to %2").arg(pid).arg(newNice));
            emit processActionTriggered();
        } else {
            QMessageBox::warning(this, "Error", "Could not change nice value. Lowering nice value requires elevated permissions.");
        }
    }
}

void ProcessView::showContextMenu(const QPoint &pos) {
    int row = m_tableWidget->currentRow();
    if (row < 0) return;

    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: #1b1f2b; color: #e0e6ed; border: 1px solid #2d3548; font-size: 13px; }"
        "QMenu::item:selected { background-color: #00d2ff; color: #0f111a; }"
    );

    menu.addAction("⚡ End Task (SIGTERM)", this, &ProcessView::onTerminateSelected);
    menu.addAction("💀 End Process", this, &ProcessView::onKillSelected);
    menu.addSeparator();
    menu.addAction("⏸ Pause Process (SIGSTOP)", this, &ProcessView::onPauseSelected);
    menu.addAction("▶ Resume Process (SIGCONT)", this, &ProcessView::onResumeSelected);
    menu.addSeparator();
    menu.addAction("⚖ Change Priority (Nice)", this, &ProcessView::onChangeNiceSelected);

    menu.exec(m_tableWidget->viewport()->mapToGlobal(pos));
}

} // namespace Harbor
