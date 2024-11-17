#include "settings.h"

#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QSettings *settings, QWidget *parent) : QDialog(parent) {
    mSettings = settings;
    this->setFixedSize(350, 150);

    // Create a layout
    mLayout = new QGridLayout();
    this->setLayout(mLayout);

    // First line
    QString prevFilePath = mSettings->value("FilePath", "").toString();
    mFilePathLabel.setText("File Path");
    mFilePathLineEdit.setText(prevFilePath);
    mFilePathLineEdit.setFocus();
    mFilePathLineEdit.setToolTip("Enter the file path to save the time tracking data, without extension");

    mCurrentDateButton.setText("Update");
    mCurrentDateButton.setToolTip("Click to use the current date as the file name");
    QString current = QDate::currentDate().toString("yyyy-MM-dd");
    QFileInfo fileInfo(mFilePathLineEdit.text());
    mCurrentDateButton.setEnabled(current != fileInfo.fileName());

    mCurrentDateButton.connect(&mCurrentDateButton, &QPushButton::clicked, [this]() {
        QString current = QDate::currentDate().toString("yyyy-MM-dd");
        QFileInfo fileInfo(mFilePathLineEdit.text());
        mFilePathLineEdit.setText(fileInfo.path() + "/" + current);
    });
    mFilePathLineEdit.connect(&mFilePathLineEdit, &QLineEdit::textChanged, [this](QString const &text) {
        QString current = QDate::currentDate().toString("yyyy-MM-dd");
        QFileInfo fileInfo(text);
        mCurrentDateButton.setEnabled(current != fileInfo.fileName());
    });

    mLayout->addWidget(&mFilePathLabel, 0, 0);
    mLayout->addWidget(&mFilePathLineEdit, 0, 1);
    mLayout->addWidget(&mCurrentDateButton, 0, 2);

    // Second line
    mTrackingIntervalLabel.setText("Interval");
    mTrackingIntervalSpinBox.setRange(1, 10);
    mTrackingIntervalSpinBox.setValue(mSettings->value("TrackingInterval", 1).toInt());
    mTrackingIntervalSpinBox.setToolTip("Enter the interval in seconds to track the time");

    mLayout->addWidget(&mTrackingIntervalLabel, 1, 0);
    mLayout->addWidget(&mTrackingIntervalSpinBox, 1, 1);

    // Third line
    mButtonBox.setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(mButtonBox.button(QDialogButtonBox::Save), &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(mButtonBox.button(QDialogButtonBox::Cancel), &QPushButton::clicked, this,
            &SettingsDialog::reject);
    mLayout->addWidget(&mButtonBox, 2, 1, 1, 2);
}

SettingsDialog::~SettingsDialog() { delete mLayout; }

bool SettingsDialog::validate() {
    // Validate the file path
    QString filePath = mFilePathLineEdit.text();
    if (filePath.isEmpty()) {
        qCritical() << "Empty: " << filePath << filePath.isEmpty();
        QMessageBox::critical(this, "Error", "File path cannot be empty");
        mFilePathLineEdit.setFocus();
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.dir().exists()) {
        qCritical() << "Exists: " << fileInfo.dir() << fileInfo.dir().exists();
        QMessageBox::critical(this, "Error", fileInfo.dir().absolutePath() + " does not exist");
        mFilePathLineEdit.setFocus();
        return false;
    }

    QFile file(filePath + ".csv");
    if (!file.exists()) {
        bool status = file.open(QIODevice::WriteOnly);
        if (!status) {
            QString msg = filePath + ".csv" + " cannot be created with error: " + file.errorString();
            qCritical() << "Error:" << msg;
            QMessageBox::critical(this, "Error", msg);
            mFilePathLineEdit.setFocus();
            return false;
        }
        file.write("Start Time,End Time,Total Time,Description\n");
        file.close();
    }

    return true;
}

void SettingsDialog::accept() {
    if (!validate())
        return;

    mSettings->setValue("FilePath", mFilePathLineEdit.text());
    mSettings->setValue("TrackingInterval", mTrackingIntervalSpinBox.value());
    QDialog::accept();
}

void SettingsDialog::reject() {
    if (!validate())
        return;

    QDialog::reject();
}
