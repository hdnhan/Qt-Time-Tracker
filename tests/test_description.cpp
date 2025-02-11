#include "description.hpp"

#include <QApplication>
#include <QtTest/QtTest>
#include <gtest/gtest.h>

class DescriptionDialogTest : public ::testing::Test {
  protected:
    static QApplication *app;

    static void SetUpTestSuite() {
        int argc = 3;
        char *argv[] = {strdup("tests"), strdup("--platform"), strdup("offscreen")};
        app = new QApplication(argc, argv);
    }

    static void TearDownTestSuite() { delete app; }
};
QApplication *DescriptionDialogTest::app = nullptr;

TEST_F(DescriptionDialogTest, DefaultPlaceholderText) {
    DescriptionDialog dialog("", nullptr);

    // Verify the placeholder text
    EXPECT_EQ(dialog.findChild<QLineEdit *>()->placeholderText(), "Enter description");
    EXPECT_EQ(dialog.findChild<QLineEdit *>()->toolTip(), "Enter the description for the time tracking");
}

TEST_F(DescriptionDialogTest, PrefilledText) {
    DescriptionDialog dialog("Test Description", nullptr);

    // Verify the prefilled text
    EXPECT_EQ(dialog.findChild<QLineEdit *>()->text(), "Test Description");
    EXPECT_EQ(dialog.getDescription(), "Test Description");
}

TEST_F(DescriptionDialogTest, UserInput) {
    DescriptionDialog dialog("", nullptr);

    QLineEdit *lineEdit = dialog.findChild<QLineEdit *>();
    ASSERT_NE(lineEdit, nullptr);

    // Simulate user typing
    QTest::keyClicks(lineEdit, "New Input");

    EXPECT_EQ(lineEdit->text(), "New Input");
}

TEST_F(DescriptionDialogTest, ButtonClickAccept) {
    DescriptionDialog dialog("", nullptr);

    // Find QPushButton by object name
    QWidget *saveButton = dialog.findChild<QWidget *>("qt_save_button");
    ASSERT_NE(saveButton, nullptr);

    // Simulate clicking Save
    QTest::mouseClick(saveButton, Qt::LeftButton);

    EXPECT_EQ(dialog.result(), QDialog::Accepted);
}

TEST_F(DescriptionDialogTest, ButtonClickReject) {
    DescriptionDialog dialog("", nullptr);

    // Find QPushButton by object name
    QWidget *cancelButton = dialog.findChild<QWidget *>("qt_cancel_button");
    ASSERT_NE(cancelButton, nullptr);

    // Simulate clicking Cancel
    QTest::mouseClick(cancelButton, Qt::LeftButton);

    EXPECT_EQ(dialog.result(), QDialog::Rejected);
}
