#pragma once

#include "settings.hpp"
#include <QDateTime>
#include <QMainWindow>
#include <QTimer>

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

  private:
    void initialize();
    void getPreviousWorkingTime();
    void updateWorkingTime();
    void startTracking();
    void stopTracking();

  private:
    QSettings *mSettings;
    SettingsDialog *mSettingsDialog;
    QDateTime mStartTime = QDateTime::currentDateTime();
    int mPreviousTotalWorkingTime = 0; // in seconds
    bool initialized = false;

    QLabel mCurrentWorkingTimeLabel;
    QLabel mTotalWorkingTimeLabel;
    QPushButton mStartButton;

    QAction *mSettingsAction, *mStopAction;

    QTimer clockTimer;    // Timer to update the clock every second
    QTimer trackingTimer; // Timer to update database when tracking
    QString mDescription;
};
