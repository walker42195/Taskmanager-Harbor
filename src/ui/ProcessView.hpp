#ifndef PROCESSVIEW_HPP
#define PROCESSVIEW_HPP

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QContextMenuEvent>
#include <vector>
#include "backend/SystemMetrics.hpp"

namespace Harbor {

class ProcessView : public QWidget {
    Q_OBJECT

public:
    explicit ProcessView(QWidget *parent = nullptr);
    ~ProcessView() override = default;

    void updateProcesses(const std::vector<ProcessInfo> &processes);

signals:
    void processActionTriggered();

private slots:
    void onSearchTextChanged(const QString &text);
    void onTerminateSelected();
    void onKillSelected();
    void onPauseSelected();
    void onResumeSelected();
    void onChangeNiceSelected();
    void showContextMenu(const QPoint &pos);

private:
    int getSelectedPid() const;
    QString getSelectedProcessName() const;

    QLineEdit *m_searchEdit;
    QTableWidget *m_tableWidget;
    QLabel *m_statusLabel;

    QPushButton *m_terminateBtn;
    QPushButton *m_killBtn;
    QPushButton *m_pauseBtn;

    std::vector<ProcessInfo> m_cachedProcesses;
    QString m_filterText;
};

} // namespace Harbor

#endif // PROCESSVIEW_HPP
