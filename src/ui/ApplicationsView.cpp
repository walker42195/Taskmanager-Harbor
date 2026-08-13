#include "ApplicationsView.hpp"
#include "backend/ProcessManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QIcon>
#include <algorithm>

namespace Harbor {

// Ties (e.g. several apps at 0 CPU%) are broken by app name so sort order
// stays deterministic across refreshes instead of jumping around when rows
// get rebuilt in a different scan order each tick.
class AppNumericItem : public QTableWidgetItem {
public:
    AppNumericItem(double numVal, const QString &appName, const QString &displayStr)
        : QTableWidgetItem(displayStr), m_numVal(numVal), m_appName(appName) {}

    bool operator<(const QTableWidgetItem &other) const override {
        const auto *o = dynamic_cast<const AppNumericItem*>(&other);
        if (o) {
            if (m_numVal != o->m_numVal) return m_numVal < o->m_numVal;
            return m_appName.localeAwareCompare(o->m_appName) < 0;
        }
        return QTableWidgetItem::operator<(other);
    }
private:
    double m_numVal{0.0};
    QString m_appName;
};

class AppTextItem : public QTableWidgetItem {
public:
    AppTextItem(const QString &displayStr, const QString &appName)
        : QTableWidgetItem(displayStr), m_appName(appName) {}

    bool operator<(const QTableWidgetItem &other) const override {
        int cmp = text().localeAwareCompare(other.text());
        if (cmp != 0) return cmp < 0;
        const auto *o = dynamic_cast<const AppTextItem*>(&other);
        if (o) return m_appName.localeAwareCompare(o->m_appName) < 0;
        return QTableWidgetItem::operator<(other);
    }
private:
    QString m_appName;
};

static QString formatAppRam(uint64_t bytes) {
    double b = static_cast<double>(bytes);
    if (b >= 1024.0 * 1024.0 * 1024.0) {
        return QString::number(b / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GiB";
    }
    return QString::number(b / (1024.0 * 1024.0), 'f', 1) + " MiB";
}

ApplicationsView::ApplicationsView(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // Top Action & Search Bar
    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍 Search applications...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color: #181c26; color: #e0e6ed; border: 1px solid #2b3245; "
        "border-radius: 6px; padding: 6px 12px; font-size: 13px; }"
        "QLineEdit:focus { border: 1px solid #00d2ff; }"
    );

    m_closeBtn = new QPushButton("Close Application", this);
    m_closeBtn->setStyleSheet(
        "QPushButton { background-color: #2b3245; color: #ffb74d; border: 1px solid #3e4860; "
        "border-radius: 6px; padding: 6px 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #3b455e; color: #ffa726; }"
    );

    m_forceQuitBtn = new QPushButton("Force Quit", this);
    m_forceQuitBtn->setStyleSheet(
        "QPushButton { background-color: #3e1b24; color: #ff5252; border: 1px solid #5c2330; "
        "border-radius: 6px; padding: 6px 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #58212f; color: #ff1744; }"
    );

    topLayout->addWidget(m_searchEdit, 2);
    topLayout->addWidget(m_closeBtn);
    topLayout->addWidget(m_forceQuitBtn);

    mainLayout->addLayout(topLayout);

    // Applications Table Widget
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels({"Application Name", "Instances", "CPU %", "Memory (RAM)", "User"});
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setIconSize(QSize(22, 22));
    m_tableWidget->setSortingEnabled(true);
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);

    m_tableWidget->setStyleSheet(
        "QTableWidget { background-color: #12141c; color: #d0d7de; gridline-color: #1e2330; border: 1px solid #242936; border-radius: 8px; }"
        "QHeaderView::section { background-color: #191d29; color: #8b949e; border: none; border-bottom: 1px solid #282e3d; padding: 8px; font-weight: bold; }"
        "QTableWidget::item:selected { background-color: #1f364d; color: #00d2ff; }"
    );

    mainLayout->addWidget(m_tableWidget);

    // Status Footer
    m_statusLabel = new QLabel("Active Applications: 0", this);
    m_statusLabel->setStyleSheet("color: #7d8590; font-size: 12px;");
    mainLayout->addWidget(m_statusLabel);

    // Connections
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ApplicationsView::onSearchTextChanged);
    connect(m_closeBtn, &QPushButton::clicked, this, &ApplicationsView::onCloseSelectedApp);
    connect(m_forceQuitBtn, &QPushButton::clicked, this, &ApplicationsView::onForceQuitSelectedApp);
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested, this, &ApplicationsView::showContextMenu);
}

void ApplicationsView::updateApplications(const std::vector<ApplicationGroup> &apps) {
    m_cachedApps = apps;

    QString selectedAppName = getSelectedAppName();

    m_tableWidget->setUpdatesEnabled(false);
    m_tableWidget->setSortingEnabled(false);
    m_tableWidget->setRowCount(0);

    int displayCount = 0;

    for (const auto &app : apps) {
        if (!m_filterText.isEmpty()) {
            QString nameStr = QString::fromStdString(app.displayName);
            if (!nameStr.contains(m_filterText, Qt::CaseInsensitive)) {
                continue;
            }
        }

        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        QString nameStr = QString::fromStdString(app.displayName);

        // App Name Item with Cached Theme Icon
        QString iconKey = QString::fromStdString(app.iconName);
        if (!m_iconCache.count(iconKey)) {
            m_iconCache[iconKey] = QIcon::fromTheme(iconKey, QIcon(":/assets/icons/applications.svg"));
        }
        QIcon appIcon = m_iconCache[iconKey];

        auto *nameItem = new QTableWidgetItem(appIcon, nameStr);
        nameItem->setFont(QFont("Cantarell", 10, QFont::DemiBold));
        nameItem->setData(Qt::UserRole, QString::fromStdString(app.displayName));
        m_tableWidget->setItem(row, 0, nameItem);

        // Process Instances Count
        int procCount = static_cast<int>(app.pids.size());
        QString procStr = (procCount == 1) ? "1 process" : QString("%1 processes").arg(procCount);
        m_tableWidget->setItem(row, 1, new AppNumericItem(procCount, nameStr, procStr));

        // CPU %
        QString cpuStr = (app.totalCpuPercent > 0.05) ? QString::number(app.totalCpuPercent, 'f', 1) + "%" : "--";
        auto *cpuItem = new AppNumericItem(app.totalCpuPercent, nameStr, cpuStr);
        if (app.totalCpuPercent > 40.0) cpuItem->setForeground(QColor(255, 82, 82));
        else if (app.totalCpuPercent > 10.0) cpuItem->setForeground(QColor(255, 183, 77));
        m_tableWidget->setItem(row, 3, cpuItem);

        // Memory (RAM)
        double ramMb = static_cast<double>(app.totalRssBytes) / (1024.0 * 1024.0);
        auto *memItem = new AppNumericItem(ramMb, nameStr, formatAppRam(app.totalRssBytes));
        memItem->setForeground(QColor(0, 230, 118));
        m_tableWidget->setItem(row, 2, memItem);

        // User
        m_tableWidget->setItem(row, 4, new AppTextItem(QString::fromStdString(app.user), nameStr));

        displayCount++;
    }

    m_tableWidget->setSortingEnabled(true);

    // Row indices shift when setSortingEnabled(true) re-sorts above, so the
    // previously-selected app must be found by name, not by the row index it
    // had while being inserted.
    if (!selectedAppName.isEmpty()) {
        for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
            auto *item = m_tableWidget->item(row, 0);
            if (item && item->data(Qt::UserRole).toString() == selectedAppName) {
                m_tableWidget->selectRow(row);
                break;
            }
        }
    }

    m_tableWidget->setUpdatesEnabled(true);
    m_statusLabel->setText(QString("Showing %1 active applications").arg(displayCount));
}

void ApplicationsView::onSearchTextChanged(const QString &text) {
    m_filterText = text.trimmed();
    updateApplications(m_cachedApps);
}

QString ApplicationsView::getSelectedAppName() const {
    int row = m_tableWidget->currentRow();
    if (row < 0) return "";
    auto *item = m_tableWidget->item(row, 0);
    if (!item) return "";
    return item->data(Qt::UserRole).toString();
}

std::vector<int> ApplicationsView::getSelectedPids() const {
    QString appName = getSelectedAppName();
    if (appName.isEmpty()) return {};

    for (const auto &app : m_cachedApps) {
        if (QString::fromStdString(app.displayName) == appName) {
            return app.pids;
        }
    }
    return {};
}

void ApplicationsView::onCloseSelectedApp() {
    std::vector<int> pids = getSelectedPids();
    if (pids.empty()) return;
    QString name = getSelectedAppName();

    bool success = true;
    for (int pid : pids) {
        if (!ProcessManager::terminateProcess(pid)) {
            success = false;
        }
    }

    if (success) {
        m_statusLabel->setText(QString("Closed application %1").arg(name));
        emit appActionTriggered();
    } else {
        m_statusLabel->setText(QString("Sent termination signal to %1").arg(name));
        emit appActionTriggered();
    }
}

void ApplicationsView::onForceQuitSelectedApp() {
    std::vector<int> pids = getSelectedPids();
    if (pids.empty()) return;
    QString name = getSelectedAppName();

    auto reply = QMessageBox::question(this, "Force Quit Application",
        QString("Are you sure you want to force quit %1 (terminating %2 processes)?").arg(name).arg(pids.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (int pid : pids) {
            ProcessManager::killProcess(pid);
        }
        m_statusLabel->setText(QString("Force quit application %1").arg(name));
        emit appActionTriggered();
    }
}

void ApplicationsView::showContextMenu(const QPoint &pos) {
    int row = m_tableWidget->currentRow();
    if (row < 0) return;

    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: #1b1f2b; color: #e0e6ed; border: 1px solid #2d3548; font-size: 13px; }"
        "QMenu::item:selected { background-color: #00d2ff; color: #0f111a; }"
    );

    menu.addAction("⚡ Close Application", this, &ApplicationsView::onCloseSelectedApp);
    menu.addAction("💀 Force Quit", this, &ApplicationsView::onForceQuitSelectedApp);

    menu.exec(m_tableWidget->viewport()->mapToGlobal(pos));
}

} // namespace Harbor
