/********************************************************************************
** Form generated from reading UI file 'ui_accelerationWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/
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
#include <QtWidgets/QWidget>
#include "QDialog"
#include "TopOptViewer.hpp"

QT_BEGIN_NAMESPACE
/****************************************************/
class Ui_AccelerationDialog
{
public:

    QVBoxLayout *labelLayout;
    QVBoxLayout *valueLayout;

    QHBoxLayout *mainLayout;
    QLabel *xAccelLabel;
    QLabel *zAccelLabel;
    QLabel *yAccelLabel;


    QLineEdit *xaccLineEdit;
    QLineEdit *yaccLineEdit;
    QLineEdit *zaccLineEdit;
    QPushButton *applyPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));


        Dialog->setMaximumWidth(GUI_MIN_WIDTH);
        mainLayout = new QHBoxLayout(Dialog);
        labelLayout = new QVBoxLayout(Dialog);
        valueLayout = new QVBoxLayout(Dialog);


        xAccelLabel = new QLabel(Dialog);
        labelLayout->addWidget(xAccelLabel);
        xaccLineEdit = new QLineEdit(Dialog);
        xaccLineEdit->setText("0");
        valueLayout->addWidget(xaccLineEdit);

        yAccelLabel = new QLabel(Dialog);
        labelLayout->addWidget(yAccelLabel);
        yaccLineEdit = new QLineEdit(Dialog);
        yaccLineEdit->setText("0");
        valueLayout->addWidget(yaccLineEdit);


        zAccelLabel = new QLabel(Dialog);
        labelLayout->addWidget(zAccelLabel);
        zaccLineEdit = new QLineEdit(Dialog);
        zaccLineEdit->setText("0");
        valueLayout->addWidget(zaccLineEdit);


        applyPushButton = new QPushButton(Dialog);
        labelLayout->addWidget(applyPushButton);

        closePushButton = new QPushButton(Dialog);
        valueLayout->addWidget(closePushButton);


        mainLayout->addLayout(labelLayout);
        mainLayout->addLayout(valueLayout);


        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Body Force", Q_NULLPTR));
        xAccelLabel->setText(QApplication::translate("Dialog", "X ", Q_NULLPTR));
        yAccelLabel->setText(QApplication::translate("Dialog", "Y ", Q_NULLPTR));
        zAccelLabel->setText(QApplication::translate("Dialog", "Z ", Q_NULLPTR));
        applyPushButton->setText(QApplication::translate("Dialog", "Apply", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Dialog", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE
/****************************************************/
class AccelerationDialog : public QDialog, public Ui_AccelerationDialog {
    Q_OBJECT

public:
    AccelerationDialog( QWidget *parent = 0);
    ~AccelerationDialog();
    void updateText();
void setViewer(TopOptViewer *v) { viewer = v;}
signals:
    void setAccelerationComplete();

private slots:
     void setAcceleration();
    void applyAcceleration();
    void closeDialog();
private:
    TopOptViewer *viewer;
    void makeConnections();

};
/****************************************************/
