#include <iterator>
#include <QApplication>
#include "ui/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Taskmanager-Harbor");
    app.setOrganizationName("Harbor");

    Harbor::MainWindow window;
    window.show();

    return app.exec();
}
