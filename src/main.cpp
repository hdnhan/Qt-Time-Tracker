#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setOrganizationName("TimeTracker");
    a.setApplicationName("TimeTracker");
    a.setWindowIcon(QIcon(":/Resources/assets/timer.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
