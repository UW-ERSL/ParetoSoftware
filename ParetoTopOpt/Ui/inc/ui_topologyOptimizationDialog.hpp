/********************************************************************************
** Form generated from reading UI file 'ui_topologyOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_TOPOLOGYOPTIMIZATIONDIALOG_H
#define UI_UI_TOPOLOGYOPTIMIZATIONDIALOG_H

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

class Ui_topologyOptimizationDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *objectiveLayout;
    QLabel *objectiveLabel;
    QComboBox *objectiveCombo;

    // use all loads
    QCheckBox *useAllLoadsCheckBox;

    QCheckBox *useSIMPMethodBox;

    QCheckBox *useTSFieldForIsoSurface;

    // use specific load
    QHBoxLayout *useLoadLayout;
    QLabel *useLoadLabel;
    QSpinBox *useLoadSpinBox;

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

        objectiveLabel = new QLabel(Dialog);
        objectiveCombo = new QComboBox;
        objectiveLayout = new QHBoxLayout(Dialog);

        useAllLoadsCheckBox = new QCheckBox(Dialog);

        useSIMPMethodBox = new QCheckBox(Dialog);
        useSIMPMethodBox->setChecked(false);

        useTSFieldForIsoSurface = new QCheckBox(Dialog);
        useTSFieldForIsoSurface->setChecked(true);


        useLoadLabel = new QLabel(Dialog);
        useLoadSpinBox = new QSpinBox(Dialog);
        useLoadSpinBox->setRange(0, 10);
        useLoadSpinBox->setSingleStep(1);
        useLoadSpinBox->setValue(0);
        useLoadLayout = new QHBoxLayout(Dialog);

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

        //add to corresponding layout
        objectiveLayout->addWidget(objectiveLabel);
        objectiveLayout->addWidget(objectiveCombo);

        useLoadLayout->addWidget(useLoadLabel);
        useLoadLayout->addWidget(useLoadSpinBox);

        desiredVolFracLayout->addWidget(desiredVolFracLabel);
        desiredVolFracLayout->addWidget(desiredVolFracSpinBox);

        stepSizeLayout->addWidget(stepSizeLabel);
        stepSizeLayout->addWidget(stepSizeSpinBox);

       //add to main layout
        mainLayout->addLayout(objectiveLayout);
        mainLayout->addWidget(useAllLoadsCheckBox);
        mainLayout->addWidget(useSIMPMethodBox);
        mainLayout->addWidget(useTSFieldForIsoSurface);
        mainLayout->addLayout(useLoadLayout);
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
        Dialog->setWindowTitle(QApplication::translate("topologyOptimizationDialog", "Optimize Topology", Q_NULLPTR));
        objectiveLabel->setText(QApplication::translate("topologyOptimizationDialog", "Objective", Q_NULLPTR));
        objectiveCombo->clear();
        objectiveCombo->insertItems(0, QStringList()
                                      << QApplication::translate("topologyOptimizationDialog", "Min. Compliance", Q_NULLPTR)
                                      << QApplication::translate("topologyOptimizationDialog", "Min. Stress", Q_NULLPTR)
                                      //<< QApplication::translate("topologyOptimizationDialog", "Max. Eigenvalue", Q_NULLPTR)
                                      );

        useAllLoadsCheckBox->setText(QApplication::translate("topologyOptimizationDialog", "Use all Loads? ", Q_NULLPTR));
        useSIMPMethodBox->setText(QApplication::translate("topologyOptimizationDialog", "Use SIMP Method? ", Q_NULLPTR));
        useTSFieldForIsoSurface->setText(QApplication::translate("topologyOptimizationDialog", "Smooth surface? ", Q_NULLPTR));

        useLoadLabel->setText(QApplication::translate("topologyOptimizationDialog", "Use Load Set", Q_NULLPTR));
        desiredVolFracLabel->setText(QApplication::translate("topologyOptimizationDialog", "Desired Vol.Frac.", Q_NULLPTR));
        stepSizeLabel->setText(QApplication::translate("topologyOptimizationDialog", "Volume Step", Q_NULLPTR));

        saveIntermediateCheckBox->setText(QApplication::translate("topologyOptimizationDialog", "Save Intermediate? ", Q_NULLPTR));
        optimizePushButton->setText(QApplication::translate("topologyOptimizationDialog", "Optimize", Q_NULLPTR));
        stopOptimizationPushButton->setText(QApplication::translate("topologyOptimizationDialog", "STOP OPTIMIZATION!", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class TopologyOptimizationDialog : public QDialog, public Ui_topologyOptimizationDialog {
    Q_OBJECT

public:
    TopologyOptimizationDialog( QWidget *parent = 0);
    ~TopologyOptimizationDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:
    void setTopOptComplete();

private slots:
    void setTopOptVariables();
    void optimize();
    void simpMessage();
    void stop();
    void closeDialog();
    void updateLoadSetCallBack();
    void onOptimizationFinished();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif // UI_UI_TOPOLOGYOPTIMIZATIONDIALOG_H
