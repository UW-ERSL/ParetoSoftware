/********************************************************************************
** Form generated from reading UI file 'ui_multiScaleOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_MULTISCALEOPTIMIZATIONDIALOG_H
#define UI_UI_MULTISCALEOPTIMIZATIONDIALOG_H

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

class Ui_multiScaleOptimizationDialog
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

    QHBoxLayout *numElemsLayout;
    QLabel *numElemsLabel;
    QSpinBox *numElemsSpinBox;

    QHBoxLayout *numClustersLayout;
    QLabel *numClustersLabel;
    QSpinBox *numClustersSpinBox;

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

        objectiveLabel = new QLabel;
        objectiveCombo = new QComboBox;
        objectiveLayout = new QHBoxLayout;

        optimizeAtMacroScaleCheckBox = new QCheckBox;
        useAllLoadsCheckBox = new QCheckBox;

        useLoadLabel = new QLabel;
        useLoadSpinBox = new QSpinBox;
        useLoadSpinBox->setRange(0, 10);
        useLoadSpinBox->setSingleStep(1);
        useLoadSpinBox->setValue(0);
        useLoadLayout = new QHBoxLayout;

        numMicroElemsLabel = new QLabel;
        numMicroElemsComboBox = new QComboBox;
        numMicroElemsLayout = new QHBoxLayout;



        // Number of elements
        numElemsLayout = new QHBoxLayout;
        numElemsLabel = new QLabel;
        numElemsSpinBox = new QSpinBox;
        numElemsSpinBox->setRange(100, 100000000);
        numElemsSpinBox->setSingleStep(1000);
        numElemsSpinBox->setValue(10000);

        numClustersLabel = new QLabel;
        numClustersSpinBox = new QSpinBox;
        numClustersSpinBox->setRange(3,20);
        numClustersSpinBox->setSingleStep(1);
        numClustersSpinBox->setValue(4);
        numClustersLayout = new QHBoxLayout;

        typeOfClusteringLabel = new QLabel;
        typeOfClusteringSpinBox = new QSpinBox;
        typeOfClusteringSpinBox->setRange(0,3);
        typeOfClusteringSpinBox->setSingleStep(1);
        typeOfClusteringSpinBox->setValue(3);
        typeOfClusteringLayout = new QHBoxLayout;


        desiredVolFracLabel = new QLabel;
        desiredVolFracSpinBox = new QDoubleSpinBox;
        desiredVolFracSpinBox->setRange(0, 1);
        desiredVolFracSpinBox->setSingleStep(0.05);
        desiredVolFracSpinBox->setValue(0.5);
        desiredVolFracLayout = new QHBoxLayout;

        stepSizeLabel = new QLabel();
        stepSizeSpinBox = new QDoubleSpinBox;
        stepSizeSpinBox->setRange(0.01,0.05);
        stepSizeSpinBox->setSingleStep(0.005);
        stepSizeSpinBox->setValue(0.025);
        stepSizeSpinBox->setDecimals(4);
        stepSizeLayout = new QHBoxLayout;

        liveUpdateCheckBox = new QCheckBox;
        saveIntermediateCheckBox = new QCheckBox;

        optimizePushButton = new QPushButton;
        stopOptimizationPushButton = new QPushButton;

        //add to corresponding layout
        objectiveLayout->addWidget(objectiveLabel);
        objectiveLayout->addWidget(objectiveCombo);

        numMicroElemsLayout->addWidget(numMicroElemsLabel);
        numMicroElemsLayout->addWidget(numMicroElemsComboBox);

        numElemsLayout->addWidget(numElemsLabel);
        numElemsLayout->addWidget(numElemsSpinBox);


        numClustersLayout->addWidget(numClustersLabel);
        numClustersLayout->addWidget(numClustersSpinBox);

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
        mainLayout->addLayout(numElemsLayout);
        mainLayout->addLayout(numClustersLayout);
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
        Dialog->setWindowTitle(QApplication::translate("multiScaleOptimizationDialog", "Optimize Topology", Q_NULLPTR));
        objectiveLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "Objective", Q_NULLPTR));
        objectiveCombo->clear();
        objectiveCombo->insertItems(0, QStringList()
                                      << QApplication::translate("multiScaleOptimizationDialog", "Min. Compliance", Q_NULLPTR)
                                      << QApplication::translate("multiScaleOptimizationDialog", "Min. Stress", Q_NULLPTR)
                                      << QApplication::translate("multiScaleOptimizationDialog", "Max. Eigenvalue", Q_NULLPTR)
                                      );

        numMicroElemsLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "#MicroElements", Q_NULLPTR));
        numMicroElemsComboBox->clear();
        numMicroElemsComboBox->insertItems(0, QStringList()
                                           << QApplication::translate("multiScaleOptimizationDialog", "8", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "125", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "512", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "1000", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "1728", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "3375", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "5832", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "8000", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "15625", Q_NULLPTR)
                                           << QApplication::translate("multiScaleOptimizationDialog", "27000", Q_NULLPTR)
                                           );

        numElemsLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "#MacroElements", Q_NULLPTR));


        numClustersLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "#Clusters", Q_NULLPTR));

        typeOfClusteringLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "Cluster Type", Q_NULLPTR));
        optimizeAtMacroScaleCheckBox->setText(QApplication::translate("multiScaleOptimizationDialog", "optimize@Macro-scale?", Q_NULLPTR));
        useAllLoadsCheckBox->setText(QApplication::translate("multiScaleOptimizationDialog", "Use all Loads? ", Q_NULLPTR));
        useLoadLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "Use Load Set", Q_NULLPTR));
        desiredVolFracLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "Desired Vol.Frac.", Q_NULLPTR));
        stepSizeLabel->setText(QApplication::translate("multiScaleOptimizationDialog", "Step Size", Q_NULLPTR));
        liveUpdateCheckBox->setText(QApplication::translate("multiScaleOptimizationDialog", "Live Update? ", Q_NULLPTR));
        saveIntermediateCheckBox->setText(QApplication::translate("multiScaleOptimizationDialog", "Save Intermediate? ", Q_NULLPTR));
        optimizePushButton->setText(QApplication::translate("multiScaleOptimizationDialog", "Optimize", Q_NULLPTR));
        stopOptimizationPushButton->setText(QApplication::translate("multiScaleOptimizationDialog", "STOP OPTIMIZATION!", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class MultiScaleOptimizationDialog : public QDialog, public Ui_multiScaleOptimizationDialog {
    Q_OBJECT

public:
    MultiScaleOptimizationDialog( QWidget *parent = 0);
    ~MultiScaleOptimizationDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:
    void setTopOptComplete();

private slots:
    void setTopOptVariables();
    void optimize();
    void stop();
    void closeDialog();
    void updateLoadSetCallBack();
    void updateMinMaxNumClusters();
    void onOptimizationFinished();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif
