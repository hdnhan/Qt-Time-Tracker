#include "mainwindow.hpp"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setOrganizationName("TimeTracker");
    a.setApplicationName("TimeTracker");
    a.setWindowIcon(QIcon(":/timer.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
