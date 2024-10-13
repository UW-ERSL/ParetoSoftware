/********************************************************************************
** Form generated from reading UI file 'ui_topologyOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_MATERIALOPTIMIZATIONDIALOG_H
#define UI_UI_MATERIALOPTIMIZATIONDIALOG_H

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
#include "ui_materialDialog.hpp"

QT_BEGIN_NAMESPACE

class Ui_materialOptimizationDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *objectiveLayout;
    QLabel *objectiveLabel;
    QComboBox *objectiveCombo;


    // material index
    QHBoxLayout *materialIndexLayout;
    QLabel *materialIndexLabel;
    QSpinBox *materialIndexSpinBox;


    QPushButton *materialSelectionPushButton;

    // desired volume fraction
    QHBoxLayout *desiredVolFracLayout;
    QLabel *desiredVolFracLabel;
    QDoubleSpinBox *desiredVolFracSpinBox;

    // step size
    QHBoxLayout *stepSizeLayout;
    QLabel *stepSizeLabel;
    QDoubleSpinBox *stepSizeSpinBox;

    QCheckBox *liveUpdateCheckBox;
    QCheckBox *saveIntermediateCheckBox;

    QPushButton *optimizePushButton;
    QPushButton *stopOptimizationPushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        mainLayout = new QVBoxLayout(Dialog);

        objectiveLabel = new QLabel(Dialog);
        objectiveCombo = new QComboBox;
        objectiveLayout = new QHBoxLayout(Dialog);

        materialIndexLabel = new QLabel(Dialog);
        materialIndexSpinBox = new QSpinBox(Dialog);
        materialIndexSpinBox->setRange(0, MAX_MATERIALS);
        materialIndexSpinBox->setSingleStep(1);
        materialIndexSpinBox->setValue(0);
        materialIndexLayout = new QHBoxLayout(Dialog);

        materialSelectionPushButton = new QPushButton(Dialog);

        desiredVolFracLabel = new QLabel(Dialog);
        desiredVolFracSpinBox = new QDoubleSpinBox(Dialog);
        desiredVolFracSpinBox->setRange(0, 1);
        desiredVolFracSpinBox->setSingleStep(0.05);
        desiredVolFracSpinBox->setValue(0.5);
        desiredVolFracLayout = new QHBoxLayout(Dialog);

        stepSizeLabel = new QLabel(Dialog);
        stepSizeSpinBox = new QDoubleSpinBox(Dialog);
        stepSizeSpinBox->setRange(0.01,0.05);
        stepSizeSpinBox->setSingleStep(0.005);
        stepSizeSpinBox->setValue(0.025);
        stepSizeSpinBox->setDecimals(4);
        stepSizeLayout = new QHBoxLayout(Dialog);

        liveUpdateCheckBox = new QCheckBox(Dialog);
        saveIntermediateCheckBox = new QCheckBox(Dialog);

        optimizePushButton = new QPushButton(Dialog);
        stopOptimizationPushButton = new QPushButton(Dialog);

        //add to corresponding layout
        objectiveLayout->addWidget(objectiveLabel);
        objectiveLayout->addWidget(objectiveCombo);

        materialIndexLayout->addWidget(materialIndexLabel);
        materialIndexLayout->addWidget(materialIndexSpinBox);

        desiredVolFracLayout->addWidget(desiredVolFracLabel);
        desiredVolFracLayout->addWidget(desiredVolFracSpinBox);

        stepSizeLayout->addWidget(stepSizeLabel);
        stepSizeLayout->addWidget(stepSizeSpinBox);

       //add to main layout
        mainLayout->addLayout(objectiveLayout);
        mainLayout->addLayout(materialIndexLayout);
        mainLayout->addWidget(materialSelectionPushButton);
        mainLayout->addLayout(desiredVolFracLayout);
        mainLayout->addLayout(stepSizeLayout);
        mainLayout->addWidget(liveUpdateCheckBox);
        mainLayout->addWidget(saveIntermediateCheckBox);
        mainLayout->addWidget(optimizePushButton);
        mainLayout->addWidget(stopOptimizationPushButton);

        liveUpdateCheckBox->hide();
        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("MaterialOptimizationDialog", "Optimize Topology", Q_NULLPTR));
        objectiveLabel->setText(QApplication::translate("MaterialOptimizationDialog", "Objective", Q_NULLPTR));
        objectiveCombo->clear();
        objectiveCombo->insertItems(0, QStringList()
                                      << QApplication::translate("MaterialOptimizationDialog", "Min. Compliance", Q_NULLPTR)
                                      );
        materialIndexLabel->setText(QApplication::translate("MaterialOptimizationDialog", "Material Index", Q_NULLPTR));

        materialSelectionPushButton->setText(QApplication::translate("MaterialOptimizationDialog", "Select Material", Q_NULLPTR));

        desiredVolFracLabel->setText(QApplication::translate("MaterialOptimizationDialog", "Desired Cost Frac.", Q_NULLPTR));
        stepSizeLabel->setText(QApplication::translate("MaterialOptimizationDialog", "Step Size", Q_NULLPTR));
        liveUpdateCheckBox->setText(QApplication::translate("MaterialOptimizationDialog", "Live Update? ", Q_NULLPTR));
        saveIntermediateCheckBox->setText(QApplication::translate("MaterialOptimizationDialog", "Save Intermediate? ", Q_NULLPTR));
        optimizePushButton->setText(QApplication::translate("MaterialOptimizationDialog", "Optimize", Q_NULLPTR));
        stopOptimizationPushButton->setText(QApplication::translate("MaterialOptimizationDialog", "STOP OPTIMIZATION!", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class MaterialOptimizationDialog : public QDialog, public Ui_materialOptimizationDialog {
    Q_OBJECT

public:
    MaterialOptimizationDialog( QWidget *parent = 0);
    ~MaterialOptimizationDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:
    void setTopOptComplete();

private slots:
    void setTopOptVariables();
    void optimize();
    void stop();
    void closeDialog();
    void openMaterialDialog();
    void updateMaterialIndexCallBack();
    void onOptimizationFinished();
    void showEvent(QShowEvent *e);

private:
    double  materialProperties[MAX_MATERIALS][8];// E,nu,Y,rho,K, Cp,alpha,$

    TopOptViewer *viewer;
    MaterialDialog  *materialDialog = nullptr;


    void makeConnections();
};

#endif // UI_UI_MATERIALOPTIMIZATIONDIALOG_H
