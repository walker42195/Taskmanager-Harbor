#ifndef APPLICATIONSVIEW_HPP
#define APPLICATIONSVIEW_HPP

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class ApplicationsView : public QWidget {
    Q_OBJECT

public:
    explicit ApplicationsView(QWidget *parent = nullptr);
    ~ApplicationsView() override = default;

    void updateApplications(const std::vector<ApplicationGroup> &apps);

signals:
    void appActionTriggered();

private slots:
    void onSearchTextChanged(const QString &text);
    void onCloseSelectedApp();
    void onForceQuitSelectedApp();
    void showContextMenu(const QPoint &pos);

private:
    std::vector<int> getSelectedPids() const;
    QString getSelectedAppName() const;

    QLineEdit *m_searchEdit;
    QTableWidget *m_tableWidget;
    QLabel *m_statusLabel;
    QPushButton *m_closeBtn;
    QPushButton *m_forceQuitBtn;

    std::vector<ApplicationGroup> m_cachedApps;
    QString m_filterText;
    std::unordered_map<QString, QIcon> m_iconCache;
};

} // namespace Harbor

#endif // APPLICATIONSVIEW_HPP
