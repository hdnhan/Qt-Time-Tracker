#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>

class DescriptionDialog : public QDialog {
  public:
    DescriptionDialog(QString msg, QWidget *parent = nullptr);
    ~DescriptionDialog();

    QString getDescription() const;

  private:
    QVBoxLayout *mLayout;
    QLineEdit mDescriptionLineEdit;
    QDialogButtonBox mButtonBox;
};