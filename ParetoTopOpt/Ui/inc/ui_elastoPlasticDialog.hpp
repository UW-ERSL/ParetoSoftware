/********************************************************************************
** Form generated from reading UI file 'ui_topologyOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ELASTOPLASTICDIALOG_H
#define UI_ELASTOPLASTICDIALOG_H

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

class Ui_elastoPlasticDialog
{
public:
    QVBoxLayout *mainLayout;

    // step size
    QHBoxLayout *stepSizeLayout;
    QLabel *stepSizeLabel;
    QSpinBox *nForceStepsSpinBox;

    QPushButton *analyzePushButton;
    QPushButton *stopAnalysisPushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        mainLayout = new QVBoxLayout(Dialog);

        stepSizeLabel = new QLabel(Dialog);
        nForceStepsSpinBox = new QSpinBox(Dialog);
        nForceStepsSpinBox->setRange(1,50);
        nForceStepsSpinBox->setSingleStep(1);
        nForceStepsSpinBox->setValue(10);
        stepSizeLayout = new QHBoxLayout(Dialog);
        stepSizeLayout->addWidget(stepSizeLabel);
        stepSizeLayout->addWidget(nForceStepsSpinBox);

        analyzePushButton = new QPushButton(Dialog);
        stopAnalysisPushButton = new QPushButton(Dialog);

       //add to main layout
        mainLayout->addLayout(stepSizeLayout);

        mainLayout->addWidget(analyzePushButton);
        mainLayout->addWidget(stopAnalysisPushButton);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("elastoPlasticDialog", "Elasto-Plastic", Q_NULLPTR));
        stepSizeLabel->setText(QApplication::translate("elastoPlasticDialog", "#Steps", Q_NULLPTR));
        analyzePushButton->setText(QApplication::translate("elastoPlasticDialog", "Analyze", Q_NULLPTR));
        stopAnalysisPushButton->setText(QApplication::translate("elastoPlasticDialog", "STOP ANALYSIS!", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class ElastoPlasticAnalysisDialog : public QDialog, public Ui_elastoPlasticDialog {
    Q_OBJECT

public:
    ElastoPlasticAnalysisDialog( QWidget *parent = 0);
    ~ElastoPlasticAnalysisDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:
    void setTopOptComplete();

private slots:
    void analyze();
    void stopAnalysis();
    void closeDialog();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif // UI_ELASTOPLASTICDIALOG_H
