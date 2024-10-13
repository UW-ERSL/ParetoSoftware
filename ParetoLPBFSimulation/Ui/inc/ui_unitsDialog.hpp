/********************************************************************************
** Form generated from reading UI file 'ui_unitsDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_UNITSDIALOG_H
#define UI_UI_UNITSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>
#include <QDialog>
#include "ParetoLPBFViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_UnitsDialog
{
public:
    QVBoxLayout *mainLayout;

    QRadioButton *MKSRadioButton;
    QRadioButton *mmKSRadioButton;
    //QRadioButton *CGSRadioButton;
    QRadioButton *IPSRadioButton;
    QHBoxLayout *unitsLayout;

    QHBoxLayout *horizontalLayout;
    QLabel *temperatureLabel;
    QComboBox *temperatureComboBox;

    QHBoxLayout *angleUnitsLayout;
    QLabel *angleUnitsLabel;
    QComboBox *angleUnitsComboBox;

    QPushButton *closePushButton;



    void setupUi(QDialog *UnitsDialog)
    {
        if (UnitsDialog->objectName().isEmpty())
            UnitsDialog->setObjectName(QStringLiteral("UnitsDialog"));

        UnitsDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(UnitsDialog);

        MKSRadioButton = new QRadioButton(UnitsDialog);
        MKSRadioButton->setChecked(true);
        mmKSRadioButton = new QRadioButton(UnitsDialog);

        IPSRadioButton = new QRadioButton(UnitsDialog);
        unitsLayout = new QHBoxLayout(UnitsDialog);
        unitsLayout->addWidget(MKSRadioButton);
        unitsLayout->addWidget(IPSRadioButton);
        unitsLayout->addWidget(mmKSRadioButton);

        //temperature
        horizontalLayout = new QHBoxLayout(UnitsDialog);
        temperatureLabel = new QLabel(UnitsDialog);
        temperatureComboBox = new QComboBox(UnitsDialog);


        horizontalLayout->addWidget(temperatureLabel);
        horizontalLayout->addWidget(temperatureComboBox);

        // angle
        angleUnitsLayout = new QHBoxLayout(UnitsDialog);
        angleUnitsLabel = new QLabel(UnitsDialog);
        angleUnitsComboBox = new QComboBox(UnitsDialog);

        angleUnitsLayout->addWidget(angleUnitsLabel);
        angleUnitsLayout->addWidget(angleUnitsComboBox);

        //close push button
        closePushButton = new QPushButton(UnitsDialog);
        //add to main layout

        mainLayout->addLayout(unitsLayout);
        mainLayout->addLayout(horizontalLayout);
        mainLayout->addLayout(angleUnitsLayout);
        mainLayout->addWidget(closePushButton);


        retranslateUi(UnitsDialog);

        QMetaObject::connectSlotsByName(UnitsDialog);
    } // setupUi

    void retranslateUi(QDialog *UnitsDialog)
    {
        UnitsDialog->setWindowTitle(QApplication::translate("UnitsDialog", "Units", Q_NULLPTR));
        MKSRadioButton->setText(QApplication::translate("UnitsDialog", "[MKS]", Q_NULLPTR));
        mmKSRadioButton->setText(QApplication::translate("UnitsDialog", "[mmKS]", Q_NULLPTR));
        IPSRadioButton->setText(QApplication::translate("UnitsDialog", "[IPS]", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("UnitsDialog", "Apply ", Q_NULLPTR));

        //temperature
        temperatureLabel->setText(QApplication::translate("UnitsDialog", "Temperature", Q_NULLPTR));
        temperatureComboBox->clear();
        temperatureComboBox->insertItems(0, QStringList()
                                         << QApplication::translate("UnitsDialog", "Celsius", Q_NULLPTR)
                                         << QApplication::translate("UnitsDialog", "Kelvin", Q_NULLPTR)
                                         << QApplication::translate("UnitsDialog", "Fahrenheit", Q_NULLPTR)
                                         );
        //angle
        angleUnitsLabel->setText(QApplication::translate("UnitsDialog", "Angle", Q_NULLPTR));
        angleUnitsComboBox->clear();
        angleUnitsComboBox->insertItems(0, QStringList()
                                         << QApplication::translate("UnitsDialog", "Degree", Q_NULLPTR)
                                         << QApplication::translate("UnitsDialog", "Radian", Q_NULLPTR)
                                         );

    } // retranslateUi

};

class UnitsDialog : public QDialog, public Ui_UnitsDialog {
    Q_OBJECT

public:
    UnitsDialog( QWidget *parent = 0);
    ~UnitsDialog();

    void setViewer(TemplateViewer *v) { viewer = v;}
signals:
    void setUnitsComplete();

private slots:
    void applyUnitsOption();
    void setTemperatureUnits();
    void setAngleUnits();

private:
    virtual void showEvent(QShowEvent * event);
    TemplateViewer *viewer;
    map<string,int>unitsMap;
    map<string,int>angleUnitsMap;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////

QT_END_NAMESPACE

#endif // UI_UI_UNITSDIALOG_H
