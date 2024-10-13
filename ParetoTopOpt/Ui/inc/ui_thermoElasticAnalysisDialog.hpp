/********************************************************************************
** Form generated from reading UI file 'ui_ThermoElasticAnalysisDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_ThermoElasticAnalysisDialog_H
#define UI_UI_ThermoElasticAnalysisDialog_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include "QDialog"
#include "TopOptViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_ThermoElasticAnalysisDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *loadLayout;

    QLabel *loadLabel;
    QSpinBox *loadSpinBox;


    QPushButton *staticThermalSolvePushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("ThermoElasticAnalysisDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        // load
        loadLabel = new QLabel;
        loadSpinBox = new QSpinBox;
        loadSpinBox->setRange(0, 10);
        loadSpinBox->setSingleStep(1);
        loadSpinBox->setValue(0);


        staticThermalSolvePushButton = new QPushButton(Dialog);


        closePushButton = new QPushButton(Dialog);
        // Layouts
        mainLayout = new QVBoxLayout(Dialog);
        loadLayout = new QHBoxLayout(Dialog);

        loadLayout->addWidget(loadLabel);
        loadLayout->addWidget(loadSpinBox);
        mainLayout->addLayout(loadLayout);
        mainLayout->addWidget(staticThermalSolvePushButton);
        mainLayout->addWidget(closePushButton);


        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("ThermoElasticAnalysisDialog", "Thermal Analysis", Q_NULLPTR));
        loadLabel->setText(QApplication::translate("ThermoElasticAnalysisDialog", "Load Set", Q_NULLPTR));
        staticThermalSolvePushButton->setText(QApplication::translate("ThermoElasticAnalysisDialog", "Thermal Analysis", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("ThermoElasticAnalysisDialog", "Close", Q_NULLPTR));

    } // retranslateUi

};


QT_END_NAMESPACE

class ThermoElasticAnalysisDialog : public QDialog, public Ui_ThermoElasticAnalysisDialog {
    Q_OBJECT
public:
    ThermoElasticAnalysisDialog( QWidget *parent = 0);
    ~ThermoElasticAnalysisDialog();
    void setViewer(TopOptViewer *v) { viewer = v;}
signals:
    void setFeaComplete();

private slots:

    void solveThermalStatic();
    void showEvent(QShowEvent *e);
    void closeThermoElasticAnalysisDialog();
    void updateLoadSetCallBack(void);
    void updateLiveVariables(void);
private:
    TopOptViewer *viewer;
    void makeConnections();
    int meshElementsMap[5];
};
////////////////////////////////////////////////////////////////////////////////

#endif // UI_UI_ThermoElasticAnalysisDialog_H
