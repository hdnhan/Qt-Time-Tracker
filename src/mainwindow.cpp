#include "mainwindow.h"
#include "description.h"
#include <QCloseEvent>
#include <QDebug>
#include <QFile>
#include <QLoggingCategory>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QTime>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle("Time Tracker");
    this->setFixedSize(300, 300 + 50);

    // Timer to update the clock every second
    connect(&clockTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::update));
    clockTimer.start(1000);

    // Timer to update Database when tracking
    connect(&trackingTimer, &QTimer::timeout, this, &MainWindow::updateWorkingTime);

    // Create a settings dialog
    mSettings = new QSettings();
    mSettingsDialog = new SettingsDialog(mSettings, this);
    qDebug() << "Settings file:" << mSettings->fileName();
    connect(mSettingsDialog, &SettingsDialog::accepted, this, &MainWindow::initialize);

    // Menu Bar
    QMenu *menu = this->menuBar()->addMenu("File");
    mSettingsAction = menu->addAction("Settings");
    mSettingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft));
    connect(mSettingsAction, &QAction::triggered, mSettingsDialog, &SettingsDialog::exec);

    mStopAction = menu->addAction("Stop");
    mStopAction->setDisabled(true);
    mStopAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(mStopAction, &QAction::triggered, this, &MainWindow::stopTracking);

    QAction *exitAction = menu->addAction("Exit");
    exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    // Current working time when the app starts
    mCurrentWorkingTimeLabel.setParent(this);
    mCurrentWorkingTimeLabel.setText("00:00"); // hours::minutes
    mCurrentWorkingTimeLabel.setStyleSheet("background-color: rgba(255, 255, 255, 0); font-size: 30px;");
    mCurrentWorkingTimeLabel.setAlignment(Qt::AlignCenter);
    mCurrentWorkingTimeLabel.setGeometry(105, 60, 90, 45);

    // Total working time including previous working time
    mTotalWorkingTimeLabel.setParent(this);
    mTotalWorkingTimeLabel.setStyleSheet("background-color: rgba(255, 255, 255, 0); font-size: 20px;");
    mTotalWorkingTimeLabel.setAlignment(Qt::AlignCenter);
    mTotalWorkingTimeLabel.setGeometry(120, 250, 60, 30);
    this->initialize();

    // Start button
    mStartButton.setParent(this);
    mStartButton.setDisabled(false);
    mStartButton.setIcon(QIcon(":/Resources/assets/start.png"));
    mStartButton.setIconSize(QSize(120, 120));
    int x = 90;
    mStartButton.setGeometry(x, x, width() - 2 * x, height() - 2 * x);
    mStartButton.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mStartButton.setStyleSheet("QPushButton { border-radius: 60px; background-color: transparent}");
    connect(&mStartButton, &QPushButton::clicked, this, &MainWindow::startTracking);
}

MainWindow::~MainWindow() {
    delete mSettings;
    delete mSettingsDialog;
    delete mSettingsAction;
    delete mStopAction;
}

/*
    Initialize mCurrentWorkingTimeLabel and mTotalWorkingTimeLabel when
    - either the app starts
    - or the user changes the file path in the settings dialog
*/
void MainWindow::initialize() {
    qDebug() << "Initializing...";
    this->getPreviousWorkingTime();

    // Update the mPreviousTotalWorkingTime
    int hours = mPreviousTotalWorkingTime / 3600;
    int minutes = (mPreviousTotalWorkingTime % 3600) / 60;
    mTotalWorkingTimeLabel.setText(
        QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')));
}

void MainWindow::startTracking() {
    qDebug() << "Start tracking";
    QString msg = "Canot start tracking.\nCheck File Path existence and permissions at Settings";
    if (!initialized) {
        qCritical() << "Error: Initialized is false";
        QMessageBox::critical(this, "Error", msg);
        return;
    }

    // Hide the start button
    mStartButton.hide();
    mStartButton.setDisabled(true);

    // Disable settings during tracking
    mSettingsAction->setDisabled(true);

    // Enable stop action during tracking
    mStopAction->setDisabled(false);

    // Add a new record to the csv file
    mStartTime = QDateTime::currentDateTime();
    QFile file(mSettings->value("FilePath").toString() + ".csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCritical() << "Failed to open file:" << file.fileName();
        QMessageBox::critical(this, "Error", msg);
        initialized = false;
        return;
    }

    // Ask the user to enter the description
    DescriptionDialog descriptionDialog(mDescription, this);
    if (descriptionDialog.exec() == QDialog::Accepted) {
        qDebug() << "Description dialog is accepted";
        mDescription = descriptionDialog.getDescription();
    }

    // Write data to the csv file (Start Time,End Time,Total Time,Description)
    QTextStream out(&file);
    out << mStartTime.toString("yyyy-MM-dd hh:mm:ss") << "," << mStartTime.toString("yyyy-MM-dd hh:mm:ss")
        << ",00:00," << mDescription << "\n";
    file.close();

    // Start tracking
    trackingTimer.start(mSettings->value("TrackingInterval", 1).toInt() * 1000 * 60);
}

void MainWindow::updateWorkingTime() {
    qDebug() << "Updating working time";
    // While working, update the current working time
    QDateTime current = QDateTime::currentDateTime();
    int totalTime = mStartTime.secsTo(current);
    int hours = totalTime / 3600;
    int minutes = (totalTime % 3600) / 60;
    QString currentTimeText = QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));
    mCurrentWorkingTimeLabel.setText(currentTimeText);

    // Update the total working time
    totalTime += mPreviousTotalWorkingTime;
    hours = totalTime / 3600;
    minutes = (totalTime % 3600) / 60;
    mTotalWorkingTimeLabel.setText(
        QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')));

    // Update the csv file
    QFile file(mSettings->value("FilePath").toString() + ".csv");
    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open file:" << file.fileName();
        QMessageBox::critical(this, "Error", "Failed to regularly update database:" + file.fileName());
        return;
    }

    // Write data to the csv file (Start Time,End Time,Total Time,Description)
    QTextStream out(&file);
    QStringList data = out.readAll().split("\n");
    file.resize(0);
    for (int i = 0; i < data.size() - 2; ++i)
        out << data[i] << "\n";
    out << mStartTime.toString("yyyy-MM-dd hh:mm:ss") << "," << current.toString("yyyy-MM-dd hh:mm:ss") << ","
        << currentTimeText << "," << mDescription << "\n";

    file.close();
}

void MainWindow::stopTracking() {
    qDebug() << "Stop tracking";
    // Ask the user to enter the description
    DescriptionDialog descriptionDialog(mDescription, this);
    if (descriptionDialog.exec() == QDialog::Accepted) {
        qDebug() << "Description dialog is accepted";
        mDescription = descriptionDialog.getDescription();
    } else {
        qDebug() << "Description dialog is rejected";
        return;
    }

    // Stop tracking
    trackingTimer.stop();

    // Enable settings after tracking
    mSettingsAction->setDisabled(false);

    // Disable stop action after tracking
    mStopAction->setDisabled(true);

    // Show the start button
    mStartButton.show();
    mStartButton.setDisabled(false);

    // Update the csv file
    QDateTime current = QDateTime::currentDateTime();
    QFile file(mSettings->value("FilePath").toString() + ".csv");
    if (!file.open(QIODevice::ReadWrite)) {
        qCritical() << "Failed to open file:" << file.fileName();
        QMessageBox::critical(this, "Error", "Failed to update when stopping tracking:" + file.fileName());
        return;
    }

    // Reset the current working time
    mCurrentWorkingTimeLabel.setText("00:00");

    // Write data to the csv file (Start Time,End Time,Total Time,Description)
    QTextStream out(&file);
    QStringList data = out.readAll().split("\n");
    file.resize(0);
    for (int i = 0; i < data.size() - 2; ++i)
        out << data[i] << "\n";
    int totalTime = mStartTime.secsTo(current);
    int hours = totalTime / 3600;
    int minutes = (totalTime % 3600) / 60;
    out << mStartTime.toString("yyyy-MM-dd hh:mm:ss") << "," << current.toString("yyyy-MM-dd hh:mm:ss") << ","
        << QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')) << ","
        << mDescription << "\n";
    file.close();
    mDescription.clear();
}

/*
Read data from the csv file (Start Time,End Time,Total Time,Description)
    - Start Time: yyyy-MM-dd hh:mm:ss
    - End Time: yyyy-MM-dd hh:mm:ss
    - Total Time: hh:mm:ss
    - Description: string

Update mPreviousTotalWorkingTime
*/
void MainWindow::getPreviousWorkingTime() {
    qDebug() << "Getting previous working time";
    QFile file(mSettings->value("FilePath").toString() + ".csv");
    if (!file.open(QIODevice::ReadOnly)) {
        // This should not happen unless the user changes the file permissions or deletes the file manually
        qCritical() << "Failed to open file:" << file.fileName();
        mPreviousTotalWorkingTime = 0;
        initialized = false;
        return;
    }

    QTextStream in(&file);
    QStringList data = in.readAll().split("\n");
    file.close();
    if (data.size() < 2) {
        mPreviousTotalWorkingTime = 0;
        return;
    }
    int previousTotalWorkingTime = 0; // in seconds
    qDebug() << data.size() - 2 << "records found";
    for (int i = 1; i < data.size() - 1; ++i) {
        QStringList row = data[i].split(",");
        qDebug() << i << row;
        QDateTime startTime = QDateTime::fromString(row[0], "yyyy-MM-dd hh:mm:ss");
        QDateTime endTime = QDateTime::fromString(row[1], "yyyy-MM-dd hh:mm:ss");
        int totalTime = startTime.secsTo(endTime);
        previousTotalWorkingTime += totalTime;
    }
    mPreviousTotalWorkingTime = previousTotalWorkingTime;
    initialized = true;
}

void MainWindow::paintEvent(QPaintEvent *) {
    // Make a square
    int side = qMin(width(), height());

    // Hour hand
    QPolygon const hourHand({QPoint(6, 7), QPoint(-6, 7), QPoint(0, -50)});
    QColor const hourHandColor(Qt::black);

    // Minute hand
    QPolygon const minuteHand({QPoint(6, 7), QPoint(-6, 7), QPoint(0, -70)});
    QColor const minuteHandColor(Qt::black);

    // Second hand
    QPolygon const secondHand({QPoint(1, 1), QPoint(-1, 1), QPoint(0, -90)});
    QColor const secondHandColor(Qt::red);

    // Create a painter
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2.0, height() / 2.0);
    painter.scale(side / 200.0, side / 200.0);
    painter.setPen(Qt::NoPen);

    // Draw background
    for (int i = 0; i < 60; ++i) {
        if (i % 5 == 0) {
            // Big ticks for hours
            painter.setPen(Qt::black);
            painter.drawLine(87, 0, 97, 0);
        } else {
            // Small ticks for minutes
            painter.setPen(Qt::gray);
            painter.drawLine(93, 0, 97, 0);
        }
        painter.rotate(6);
    }

    // Draw the clock hands
    std::function<void(QPolygon const &, QColor const &, float)> drawHand =
        [&](QPolygon const &hand, QColor const &color, float angle) {
            painter.setBrush(color);
            painter.save();
            painter.rotate(angle);
            painter.drawConvexPolygon(hand);
            painter.restore();
        };

    // Get the current time
    QTime current = QTime::currentTime();

    // Draw the hour, minute, and second hands
    drawHand(hourHand, hourHandColor, 30.0 * (current.hour() + current.minute() / 60.0));
    drawHand(minuteHand, minuteHandColor, 6.0 * (current.minute() + current.second() / 60.0));
    drawHand(secondHand, secondHandColor, 6.0 * current.second());

    painter.end(); // End the painter
}

void MainWindow::closeEvent(QCloseEvent *event) {
    qDebug() << "Closing...";
    if (trackingTimer.isActive()) {
        this->stopTracking();
        if (trackingTimer.isActive())
            event->ignore();
        else
            event->accept();
    }
}
