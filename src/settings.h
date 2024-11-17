#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>

class SettingsDialog : public QDialog {
  public:
    SettingsDialog(QSettings *settings, QWidget *parent = nullptr);
    ~SettingsDialog();

  private:
    void accept() override;
    void reject() override;

  private:
    bool validate();

  private:
    QSettings *mSettings;
    QGridLayout *mLayout;
    // First line
    QLabel mFilePathLabel;
    QLineEdit mFilePathLineEdit;
    QPushButton mCurrentDateButton;
    // Second line
    QLabel mTrackingIntervalLabel;
    QSpinBox mTrackingIntervalSpinBox;
    // Third line
    QDialogButtonBox mButtonBox;
};
