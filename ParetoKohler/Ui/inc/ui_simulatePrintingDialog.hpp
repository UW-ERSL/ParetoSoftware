#pragma once

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include<QtWidgets/QVBoxLayout>
#include<QtWidgets/QHBoxLayout>

#include "QDialog"
#include "TemplateViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_simulatePrintingDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *(propertyLayout[5]);
    QLabel *(propertyLabel[5]);
    QLineEdit *(propertyLineEdit[5]);

    QPushButton *simulateMoisturePushButton;
    QPushButton *simulateMoistureAndGravityPushButton;
    QPushButton *stopPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *PrintingDialog)
    {
        if (PrintingDialog->objectName().isEmpty())
            PrintingDialog->setObjectName(QStringLiteral("Printing"));

        PrintingDialog->setMinimumWidth(GUI_MIN_WIDTH);

        mainLayout = new QVBoxLayout(PrintingDialog);

        propertyLayout[0] = new QHBoxLayout(PrintingDialog);
        propertyLabel[0] = new QLabel(PrintingDialog);
        propertyLineEdit[0] = new QLineEdit(PrintingDialog);
        propertyLabel[0]->setMinimumWidth(150);
        propertyLineEdit[0]->setMaximumWidth(120);
        propertyLineEdit[0]->setText(QString::number(22.5));
        propertyLabel[0]->setText(QApplication::translate("Printing", "Initial clay moisture (%)", Q_NULLPTR));
        propertyLayout[0]->addWidget(propertyLabel[0]);
        propertyLayout[0]->addWidget(propertyLineEdit[0]);
        mainLayout->addLayout(propertyLayout[0]);

        propertyLayout[1] = new QHBoxLayout(PrintingDialog);
        propertyLabel[1] = new QLabel(PrintingDialog);
        propertyLineEdit[1] = new QLineEdit(PrintingDialog);
        propertyLabel[1]->setMinimumWidth(150);
        propertyLineEdit[1]->setMaximumWidth(120);
        propertyLineEdit[1]->setText(QString::number(65));
        propertyLabel[1]->setText(QApplication::translate("Printing", "Relative humidity (%)", Q_NULLPTR));
        propertyLayout[1]->addWidget(propertyLabel[1]);
        propertyLayout[1]->addWidget(propertyLineEdit[1]);
        mainLayout->addLayout(propertyLayout[1]);

        propertyLayout[2] = new QHBoxLayout(PrintingDialog);
        propertyLabel[2] = new QLabel(PrintingDialog);
        propertyLineEdit[2] = new QLineEdit(PrintingDialog);
        propertyLabel[2]->setMinimumWidth(150);
        propertyLineEdit[2]->setMaximumWidth(120);
        propertyLineEdit[2]->setText(QString::number(5));
        propertyLabel[2]->setText(QApplication::translate("Printing", "h-Print (*1e-5 m/s)", Q_NULLPTR));
        propertyLayout[2]->addWidget(propertyLabel[2]);
        propertyLayout[2]->addWidget(propertyLineEdit[2]);
        mainLayout->addLayout(propertyLayout[2]);

        propertyLayout[3] = new QHBoxLayout(PrintingDialog);
        propertyLabel[3] = new QLabel(PrintingDialog);
        propertyLineEdit[3] = new QLineEdit(PrintingDialog);
        propertyLabel[3]->setMinimumWidth(150);
        propertyLineEdit[3]->setMaximumWidth(120);
        propertyLineEdit[3]->setText(QString::number(0.25));
        propertyLabel[3]->setText(QApplication::translate("Printing", "beta (E = E0*exp(-beta*m))", Q_NULLPTR));
        propertyLayout[3]->addWidget(propertyLabel[3]);
        propertyLayout[3]->addWidget(propertyLineEdit[3]);
        mainLayout->addLayout(propertyLayout[3]);

        propertyLayout[4] = new QHBoxLayout(PrintingDialog);
        propertyLabel[4] = new QLabel(PrintingDialog);
        propertyLineEdit[4] = new QLineEdit(PrintingDialog);
        propertyLabel[4]->setMinimumWidth(150);
        propertyLineEdit[4]->setMaximumWidth(120);
        propertyLineEdit[4]->setText(QString::number(50));
        propertyLabel[4]->setText(QApplication::translate("Printing", "Printing steps ", Q_NULLPTR));
        propertyLayout[4]->addWidget(propertyLabel[4]);
        propertyLayout[4]->addWidget(propertyLineEdit[4]);
        mainLayout->addLayout(propertyLayout[4]);

        simulateMoisturePushButton = new QPushButton(PrintingDialog);
        mainLayout->addWidget(simulateMoisturePushButton);

        simulateMoistureAndGravityPushButton = new QPushButton(PrintingDialog);
        mainLayout->addWidget(simulateMoistureAndGravityPushButton);
        simulateMoistureAndGravityPushButton->hide();

        stopPushButton = new QPushButton(PrintingDialog);
        mainLayout->addWidget(stopPushButton);

        closePushButton = new QPushButton(PrintingDialog);
        mainLayout->addWidget(closePushButton);

        QMetaObject::connectSlotsByName(PrintingDialog);

        retranslateUi(PrintingDialog);
    } // setupUi

    void retranslateUi(QDialog *PrintingDialog)
    {
        PrintingDialog->setWindowTitle(QApplication::translate("Simulation", "Printing", Q_NULLPTR));
        simulateMoisturePushButton->setText(QApplication::translate("Printing", "Simulate Printing", Q_NULLPTR));
        simulateMoistureAndGravityPushButton->setText(QApplication::translate("Printing", "Simulate Printing+Gravity", Q_NULLPTR));
        stopPushButton->setText(QApplication::translate("Printing", "Stop Simulation", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Printing", "Close", Q_NULLPTR));
    } // retranslateUi

};
QT_END_NAMESPACE
////////////////////////////////////////////////////////////////////////////////
class PrintingDialog : public QDialog, public Ui_simulatePrintingDialog{
    Q_OBJECT

public:
    PrintingDialog( QWidget *parent = 0);
    ~PrintingDialog();
void setViewer(TemplateViewer *v) { viewer = v; }

signals:
    void setMoistureComplete();
private slots:
    void simulatePrinting();
    void simulateMoisture();
    void simulateMoistureAndGravity();
    void stopSimulation();
    void closeDialog();

private:
    TemplateViewer *viewer;
    bool    continueSimulation;
    void    makeConnections();

};
////////////////////////////////////////////////////////////////////////////////

