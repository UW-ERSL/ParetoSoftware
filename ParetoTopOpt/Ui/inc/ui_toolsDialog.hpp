#pragma once

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <string>
#include <thread>
#include <chrono>
#include <QtWidgets/QWidget>
#include "QDialog"
#include "TopOptViewer.hpp"


QT_BEGIN_NAMESPACE
/****************************************************/
class Ui_ToolsDialog
{
public:

    QVBoxLayout *mainLayout;

    QPushButton *captureVideoPushButton;
    QPushButton *saveScreenShotPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(200);
        mainLayout = new QVBoxLayout(Dialog);

        saveScreenShotPushButton = new QPushButton(Dialog);
        closePushButton = new QPushButton(Dialog);
        captureVideoPushButton= new QPushButton(Dialog);

        mainLayout->addWidget(saveScreenShotPushButton);
        mainLayout->addWidget(captureVideoPushButton);
        mainLayout->addWidget(closePushButton);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Tools", Q_NULLPTR));

        captureVideoPushButton->setText(QApplication::translate("Dialog", "Video", Q_NULLPTR));
        saveScreenShotPushButton->setText(QApplication::translate("Dialog", "Screenshot", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Dialog", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE
/****************************************************/
class ToolsDialog : public QDialog, public Ui_ToolsDialog {
    Q_OBJECT

public:
    ToolsDialog( QWidget *parent = 0);
    ~ToolsDialog();

    void setViewer(TopOptViewer *v) { viewer = v;}

private slots:
    void captureScreenShot();
    void captureScreenVideo(double captureFrequency = -1 ); // give a negative arg when calling from another func which is already rec a video
    void closeDialog();
private:
    TopOptViewer *viewer;
    void makeConnections();

};
/****************************************************/
