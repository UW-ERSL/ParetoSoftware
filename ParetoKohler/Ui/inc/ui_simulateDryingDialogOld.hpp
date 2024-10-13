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

class Ui_moistureDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *(propertyLayout[7]);
    QLabel *(propertyLabel[7]);
    QLineEdit *(propertyLineEdit[7]);

    QPushButton *simulatePrintingPushButton;
    QPushButton *simulateDryingPushButton;

    QCheckBox   *includeGravityCheckBox;
    QCheckBox   *includeShrinkingCheckBox;
    QPushButton *stopPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *MoistureDialog)
    {
        if (MoistureDialog->objectName().isEmpty())
            MoistureDialog->setObjectName(QStringLiteral("Simulation"));

        MoistureDialog->setMinimumWidth(GUI_MIN_WIDTH);

        mainLayout = new QVBoxLayout(MoistureDialog);

        propertyLayout[0] = new QHBoxLayout(MoistureDialog);
        propertyLabel[0] = new QLabel(MoistureDialog);
        propertyLineEdit[0] = new QLineEdit(MoistureDialog);
        propertyLabel[0]->setMinimumWidth(150);
        propertyLineEdit[0]->setMaximumWidth(120);
        propertyLineEdit[0]->setText(QString::number(22.5));
        propertyLabel[0]->setText(QApplication::translate("Simulation", "Initial clay moisture (%)", Q_NULLPTR));
        propertyLayout[0]->addWidget(propertyLabel[0]);
        propertyLayout[0]->addWidget(propertyLineEdit[0]);
        mainLayout->addLayout(propertyLayout[0]);

        propertyLayout[1] = new QHBoxLayout(MoistureDialog);
        propertyLabel[1] = new QLabel(MoistureDialog);
        propertyLineEdit[1] = new QLineEdit(MoistureDialog);
        propertyLabel[1]->setMinimumWidth(150);
        propertyLineEdit[1]->setMaximumWidth(120);
        propertyLineEdit[1]->setText(QString::number(65));
        propertyLabel[1]->setText(QApplication::translate("Simulation", "Relative humidity (%)", Q_NULLPTR));
        propertyLayout[1]->addWidget(propertyLabel[1]);
        propertyLayout[1]->addWidget(propertyLineEdit[1]);
        mainLayout->addLayout(propertyLayout[1]);

        propertyLayout[2] = new QHBoxLayout(MoistureDialog);
        propertyLabel[2] = new QLabel(MoistureDialog);
        propertyLineEdit[2] = new QLineEdit(MoistureDialog);
        propertyLabel[2]->setMinimumWidth(150);
        propertyLineEdit[2]->setMaximumWidth(120);
        propertyLineEdit[2]->setText(QString::number(1));
        propertyLabel[2]->setText(QApplication::translate("Simulation", "h-Print (*1e-4 m/s)", Q_NULLPTR));
        propertyLayout[2]->addWidget(propertyLabel[2]);
        propertyLayout[2]->addWidget(propertyLineEdit[2]);
        mainLayout->addLayout(propertyLayout[2]);

        propertyLayout[3] = new QHBoxLayout(MoistureDialog);
        propertyLabel[3] = new QLabel(MoistureDialog);
        propertyLineEdit[3] = new QLineEdit(MoistureDialog);
        propertyLabel[3]->setMinimumWidth(150);
        propertyLineEdit[3]->setMaximumWidth(120);
        propertyLineEdit[3]->setText(QString::number(1));
        propertyLabel[3]->setText(QApplication::translate("Simulation", "h-Dry (*1e-4 m/s)", Q_NULLPTR));
        propertyLayout[3]->addWidget(propertyLabel[3]);
        propertyLayout[3]->addWidget(propertyLineEdit[3]);
        mainLayout->addLayout(propertyLayout[3]);

        propertyLayout[4] = new QHBoxLayout(MoistureDialog);
        propertyLabel[4] = new QLabel(MoistureDialog);
        propertyLineEdit[4] = new QLineEdit(MoistureDialog);
        propertyLabel[4]->setMinimumWidth(150);
        propertyLineEdit[4]->setMaximumWidth(120);
        propertyLineEdit[4]->setText(QString::number(3));
        propertyLabel[4]->setText(QApplication::translate("Simulation", "Drying time (hrs) ", Q_NULLPTR));
        propertyLayout[4]->addWidget(propertyLabel[4]);
        propertyLayout[4]->addWidget(propertyLineEdit[4]);
        mainLayout->addLayout(propertyLayout[4]);

        propertyLayout[5] = new QHBoxLayout(MoistureDialog);
        propertyLabel[5] = new QLabel(MoistureDialog);
        propertyLineEdit[5] = new QLineEdit(MoistureDialog);
        propertyLabel[5]->setMinimumWidth(150);
        propertyLineEdit[5]->setMaximumWidth(120);
        propertyLineEdit[5]->setText(QString::number(20));
        propertyLabel[5]->setText(QApplication::translate("Simulation", "Printing steps ", Q_NULLPTR));
        propertyLayout[5]->addWidget(propertyLabel[5]);
        propertyLayout[5]->addWidget(propertyLineEdit[5]);
        mainLayout->addLayout(propertyLayout[5]);

        propertyLayout[6] = new QHBoxLayout(MoistureDialog);
        propertyLabel[6] = new QLabel(MoistureDialog);
        propertyLineEdit[6] = new QLineEdit(MoistureDialog);
        propertyLabel[6]->setMinimumWidth(150);
        propertyLineEdit[6]->setMaximumWidth(120);
        propertyLineEdit[6]->setText(QString::number(36));
        propertyLabel[6]->setText(QApplication::translate("Simulation", "Drying steps ", Q_NULLPTR));
        propertyLayout[6]->addWidget(propertyLabel[6]);
        propertyLayout[6]->addWidget(propertyLineEdit[6]);
        mainLayout->addLayout(propertyLayout[6]);

        includeGravityCheckBox = new QCheckBox(MoistureDialog);
        mainLayout->addWidget(includeGravityCheckBox);

        includeShrinkingCheckBox = new QCheckBox(MoistureDialog);
        mainLayout->addWidget(includeShrinkingCheckBox);
        includeShrinkingCheckBox->hide();

        simulatePrintingPushButton = new QPushButton(MoistureDialog);
        mainLayout->addWidget(simulatePrintingPushButton);

        simulateDryingPushButton = new QPushButton(MoistureDialog);
        mainLayout->addWidget(simulateDryingPushButton);

        stopPushButton = new QPushButton(MoistureDialog);
        mainLayout->addWidget(stopPushButton);

        closePushButton = new QPushButton(MoistureDialog);
        mainLayout->addWidget(closePushButton);

        QMetaObject::connectSlotsByName(MoistureDialog);

        retranslateUi(MoistureDialog);
    } // setupUi

    void retranslateUi(QDialog *MoistureDialog)
    {
        MoistureDialog->setWindowTitle(QApplication::translate("Simulation", "Simulation", Q_NULLPTR));
        simulatePrintingPushButton->setText(QApplication::translate("Simulation", "Simulate Printing", Q_NULLPTR));
        simulateDryingPushButton->setText(QApplication::translate("Simulation", "Simulate Drying", Q_NULLPTR));
        stopPushButton->setText(QApplication::translate("Simulation", "Stop Simulation", Q_NULLPTR));
        includeGravityCheckBox->setText(QApplication::translate("Simulation", "Include Gravity", Q_NULLPTR));
        includeShrinkingCheckBox->setText(QApplication::translate("Simulation", "Include Shrinking", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Simulation", "Close", Q_NULLPTR));
    } // retranslateUi

};
QT_END_NAMESPACE
////////////////////////////////////////////////////////////////////////////////
class MoistureDialog : public QDialog, public Ui_moistureDialog{
    Q_OBJECT

public:
    MoistureDialog( QWidget *parent = 0);
    ~MoistureDialog();
void setViewer(TemplateViewer *v) { viewer = v; }


signals:
    void setMoistureComplete();
private slots:
    void simulatePrinting();
    void simulateDrying();
    void stopSimulation();
    void closeDialog();

private:
    TemplateViewer *viewer;
    bool    continueSimulation;
    void    makeConnections();

};
////////////////////////////////////////////////////////////////////////////////

