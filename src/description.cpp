#include "description.h"

#include <QPushButton>

DescriptionDialog::DescriptionDialog(QString msg, QWidget *parent) : QDialog(parent) {
    this->setFixedSize(350, 120);

    mLayout = new QVBoxLayout();
    this->setLayout(mLayout);

    if (msg.isEmpty())
        mDescriptionLineEdit.setPlaceholderText("Enter description");
    else
        mDescriptionLineEdit.setText(msg);
    mDescriptionLineEdit.setMinimumHeight(40);
    mDescriptionLineEdit.setToolTip("Enter the description for the time tracking");
    mDescriptionLineEdit.setFocus();
    mLayout->addWidget(&mDescriptionLineEdit);
    mLayout->addSpacing(20);

    mButtonBox.setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(mButtonBox.button(QDialogButtonBox::Save), &QPushButton::clicked, this,
            &DescriptionDialog::accept);
    connect(mButtonBox.button(QDialogButtonBox::Cancel), &QPushButton::clicked, this,
            &DescriptionDialog::reject);
    mLayout->addWidget(&mButtonBox);
}

DescriptionDialog::~DescriptionDialog() { delete mLayout; }

QString DescriptionDialog::getDescription() const { return mDescriptionLineEdit.text(); }