/********************************************************************************
** Form generated from reading UI file 'ui_thermalOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_THERMALOPTIMIZATIONDIALOG_H
#define UI_UI_THERMALOPTIMIZATIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

#include <QDialog>
#include <QColorDialog>
#include "TopOptViewer.hpp"


QT_BEGIN_NAMESPACE

class Ui_thermalOptimizationDialog
{
public:
    QVBoxLayout *mainLayout;


    // desired volume fraction
    QHBoxLayout *desiredVolFracLayout;
    QLabel *desiredVolFracLabel;
    QDoubleSpinBox *desiredVolFracSpinBox;

    // volume step size
    QHBoxLayout *stepSizeLayout;
    QLabel *stepSizeLabel;
    QDoubleSpinBox *stepSizeSpinBox;

    QCheckBox *saveIntermediateCheckBox;

    QPushButton *optimizePushButton;
    QPushButton *stopOptimizationPushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        mainLayout = new QVBoxLayout(Dialog);


        desiredVolFracLabel = new QLabel(Dialog);
        desiredVolFracSpinBox = new QDoubleSpinBox(Dialog);
        desiredVolFracSpinBox->setRange(0, 1);
        desiredVolFracSpinBox->setSingleStep(0.05);
        desiredVolFracSpinBox->setValue(0.5);
        desiredVolFracLayout = new QHBoxLayout(Dialog);

        stepSizeLabel = new QLabel(Dialog);
        stepSizeSpinBox = new QDoubleSpinBox(Dialog);
        stepSizeSpinBox->setRange(0.01,0.5);
        stepSizeSpinBox->setSingleStep(0.005);
        stepSizeSpinBox->setValue(0.025);
        stepSizeSpinBox->setDecimals(5);
        stepSizeLayout = new QHBoxLayout(Dialog);
        saveIntermediateCheckBox = new QCheckBox(Dialog);

        optimizePushButton = new QPushButton(Dialog);
        stopOptimizationPushButton = new QPushButton(Dialog);


        desiredVolFracLayout->addWidget(desiredVolFracLabel);
        desiredVolFracLayout->addWidget(desiredVolFracSpinBox);

        stepSizeLayout->addWidget(stepSizeLabel);
        stepSizeLayout->addWidget(stepSizeSpinBox);

       //add to main layout
        mainLayout->addLayout(desiredVolFracLayout);
        mainLayout->addLayout(stepSizeLayout);

        mainLayout->addWidget(saveIntermediateCheckBox);
        mainLayout->addWidget(optimizePushButton);
        mainLayout->addWidget(stopOptimizationPushButton);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("thermalOptimizationDialog", "Optimize Topology", Q_NULLPTR));

        desiredVolFracLabel->setText(QApplication::translate("thermalOptimizationDialog", "Desired Vol.Frac.", Q_NULLPTR));
        stepSizeLabel->setText(QApplication::translate("thermalOptimizationDialog", "Volume Step", Q_NULLPTR));

        saveIntermediateCheckBox->setText(QApplication::translate("thermalOptimizationDialog", "Save Intermediate? ", Q_NULLPTR));
        optimizePushButton->setText(QApplication::translate("thermalOptimizationDialog", "Optimize", Q_NULLPTR));
        stopOptimizationPushButton->setText(QApplication::translate("thermalOptimizationDialog", "STOP OPTIMIZATION!", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class ThermalOptimizationDialog : public QDialog, public Ui_thermalOptimizationDialog {
    Q_OBJECT

public:
    ThermalOptimizationDialog( QWidget *parent = 0);
    ~ThermalOptimizationDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:
    void setTopOptComplete();

private slots:
    void setTopOptVariables();
    void optimize();
    void stop();
    void closeDialog();
    void updateLoadSetCallBack();
    void onOptimizationFinished();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif // UI_UI_THERMALOPTIMIZATIONDIALOG_H
