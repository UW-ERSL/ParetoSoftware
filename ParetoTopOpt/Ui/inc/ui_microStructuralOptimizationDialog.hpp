/********************************************************************************
** Form generated from reading UI file 'ui_microStructuralOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_MICROSTRUCTURALOPTIMIZATIONDIALOG_H
#define UI_UI_MICROSTRUCTURALOPTIMIZATIONDIALOG_H

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

class Ui_microStructuralOptimizationDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *objectiveLayout;
    QLabel *objectiveLabel;
    QComboBox *objectiveCombo;

    // perform two scale optimization(include macro-scale?)
    QCheckBox *optimizeAtMacroScaleCheckBox;

    // use all loads
    QCheckBox *useAllLoadsCheckBox;


    // use specific load
    QHBoxLayout *useLoadLayout;
    QLabel *useLoadLabel;
    QSpinBox *useLoadSpinBox;

    QHBoxLayout *numMicroElemsLayout;
    QLabel *numMicroElemsLabel;
    QComboBox *numMicroElemsComboBox;

    QHBoxLayout *numGroupsLayout;
    QLabel *numGroupsLabel;
    QSpinBox *numGroupsSpinBox;

    QHBoxLayout *typeOfClusteringLayout;
    QLabel *typeOfClusteringLabel;
    QSpinBox *typeOfClusteringSpinBox;

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

        optimizeAtMacroScaleCheckBox = new QCheckBox(Dialog);        
        useAllLoadsCheckBox = new QCheckBox(Dialog);

        useLoadLabel = new QLabel(Dialog);
        useLoadSpinBox = new QSpinBox(Dialog);
        useLoadSpinBox->setRange(0, 10);
        useLoadSpinBox->setSingleStep(1);
        useLoadSpinBox->setValue(0);
        useLoadLayout = new QHBoxLayout(Dialog);

        numMicroElemsLabel = new QLabel(Dialog);
        numMicroElemsComboBox = new QComboBox(Dialog);
        numMicroElemsLayout = new QHBoxLayout(Dialog);

        numGroupsLabel = new QLabel(Dialog);
        numGroupsSpinBox = new QSpinBox(Dialog);
        numGroupsSpinBox->setRange(1,20);
        numGroupsSpinBox->setSingleStep(1);
        numGroupsSpinBox->setValue(4);
        numGroupsLayout = new QHBoxLayout(Dialog);

        typeOfClusteringLabel = new QLabel(Dialog);
        typeOfClusteringSpinBox = new QSpinBox(Dialog);
        typeOfClusteringSpinBox->setRange(0,3);
        typeOfClusteringSpinBox->setSingleStep(1);
        typeOfClusteringSpinBox->setValue(1);
        typeOfClusteringLayout = new QHBoxLayout(Dialog);


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

        numMicroElemsLayout->addWidget(numMicroElemsLabel);
        numMicroElemsLayout->addWidget(numMicroElemsComboBox);

        numGroupsLayout->addWidget(numGroupsLabel);
        numGroupsLayout->addWidget(numGroupsSpinBox);

        typeOfClusteringLayout->addWidget(typeOfClusteringLabel);
        typeOfClusteringLayout->addWidget(typeOfClusteringSpinBox);

        useLoadLayout->addWidget(useLoadLabel);
        useLoadLayout->addWidget(useLoadSpinBox);

        desiredVolFracLayout->addWidget(desiredVolFracLabel);
        desiredVolFracLayout->addWidget(desiredVolFracSpinBox);

        stepSizeLayout->addWidget(stepSizeLabel);
        stepSizeLayout->addWidget(stepSizeSpinBox);

       //add to main layout
        mainLayout->addLayout(objectiveLayout);
        mainLayout->addLayout(numMicroElemsLayout);
        mainLayout->addLayout(numGroupsLayout);
        mainLayout->addLayout(typeOfClusteringLayout);
        mainLayout->addWidget(optimizeAtMacroScaleCheckBox);
        mainLayout->addWidget(useAllLoadsCheckBox);        
        mainLayout->addLayout(useLoadLayout);
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
        Dialog->setWindowTitle(QApplication::translate("microStructuralOptimizationDialog", "Optimize Topology", Q_NULLPTR));
        objectiveLabel->setText(QApplication::translate("microStructuralOptimizationDialog", "Objective", Q_NULLPTR));
        objectiveCombo->clear();
        objectiveCombo->insertItems(0, QStringList()
                                      << QApplication::translate("microStructuralOptimizationDialog", "Min. Compliance", Q_NULLPTR)
                                      << QApplication::translate("microStructuralOptimizationDialog", "Min. Stress", Q_NULLPTR)
                                      << QApplication::translate("microStructuralOptimizationDialog", "Max. Eigenvalue", Q_NULLPTR)
                                      );

        numMicroElemsLabel->setText(QApplication::translate("microStructuralOptimizationDialog", "#MicroElements", Q_NULLPTR));
        numMicroElemsComboBox->clear();
        numMicroElemsComboBox->insertItems(0, QStringList()
                                           << QApplication::translate("microStructuralOptimizationDialog", "8", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "125", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "512", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "1000", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "1728", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "3375", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "5832", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "8000", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "15625", Q_NULLPTR)
                                           << QApplication::translate("microStructuralOptimizationDialog", "27000", Q_NULLPTR)
                                           );

        numGroupsLabel->setText(QApplication::translate("microStructuralOptimizationDialog", "#Groups", Q_NULLPTR));

        typeOfClusteringLabel->setText(QApplication::translate("microStructuralOptimizationDialog", "Cluster Type", Q_NULLPTR));        
        optimizeAtMacroScaleCheckBox->setText(QApplication::translate("microStructuralOptimizationDialog", "optimize@Macro-scale?", Q_NULLPTR));
        useAllLoadsCheckBox->setText(QApplication::translate("microStructuralOptimizationDialog", "Use all Loads? ", Q_NULLPTR));
        useLoadLabel->setText(QApplication::translate("microStructuralOptimizationDialog", "Use Load Set", Q_NULLPTR));
        desiredVolFracLabel->setText(QApplication::translate("microStructuralOptimizationDialog", "Desired Vol.Frac.", Q_NULLPTR));
        stepSizeLabel->setText(QApplication::translate("microStructuralOptimizationDialog", "Step Size", Q_NULLPTR));
        liveUpdateCheckBox->setText(QApplication::translate("microStructuralOptimizationDialog", "Live Update? ", Q_NULLPTR));
        saveIntermediateCheckBox->setText(QApplication::translate("microStructuralOptimizationDialog", "Save Intermediate? ", Q_NULLPTR));
        optimizePushButton->setText(QApplication::translate("microStructuralOptimizationDialog", "Optimize", Q_NULLPTR));
        stopOptimizationPushButton->setText(QApplication::translate("microStructuralOptimizationDialog", "STOP OPTIMIZATION!", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class MicroStructuralOptimizationDialog : public QDialog, public Ui_microStructuralOptimizationDialog {
    Q_OBJECT

public:
    MicroStructuralOptimizationDialog( QWidget *parent = 0);
    ~MicroStructuralOptimizationDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:
    void setTopOptComplete();

private slots:
    void setTopOptVariables();
    void optimize();
    void runMicroStructuralOptimization();
    void stop();
    void closeDialog();
    void updateLoadSetCallBack();
    void onOptimizationFinished();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif
