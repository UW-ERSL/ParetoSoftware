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
#include <QtWidgets/QPushButton>
#include<QtWidgets/QVBoxLayout>
#include<QtWidgets/QHBoxLayout>
#include <QDialog>
#include <QColorDialog>
#include "LatticeViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_thermalLoadWindow
{
public:
    QHBoxLayout *mainLayout;

    QVBoxLayout *thermalValueLayout;

    QVBoxLayout *thermalLabelLayout;

    QLabel *thermalLoadLabel;

    QPushButton *applyThermalLoads;



    QComboBox *thermalLoadsComboBox;
    QLabel *thermalInputFieldLabel1;
    QLabel *thermalInputFieldLabel2;

    QLineEdit *thermalLineEdit1;
    QLineEdit *thermalLineEdit2;

    QPushButton *closePushButton;


    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("LoadsWindow"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QHBoxLayout(Dialog);

        //thermal loads
        thermalLoadLabel = new QLabel(Dialog);

        thermalLabelLayout = new QVBoxLayout(Dialog);
        thermalLabelLayout->addWidget(thermalLoadLabel);;

        thermalValueLayout = new QVBoxLayout(Dialog);

        thermalLoadsComboBox = new QComboBox(Dialog);
        thermalValueLayout->addWidget(thermalLoadsComboBox);

        thermalInputFieldLabel1 = new QLabel(Dialog);
        thermalLabelLayout->addWidget(thermalInputFieldLabel1);

        thermalLineEdit1 = new QLineEdit(Dialog);
        thermalValueLayout->addWidget(thermalLineEdit1);

        thermalInputFieldLabel2 = new QLabel(Dialog);
        thermalLabelLayout->addWidget(thermalInputFieldLabel2);

        thermalLineEdit2 = new QLineEdit(Dialog);
        thermalValueLayout->addWidget(thermalLineEdit2);

        applyThermalLoads = new QPushButton(Dialog);
        thermalLabelLayout->addWidget(applyThermalLoads);

        closePushButton = new QPushButton(Dialog);
        thermalValueLayout->addWidget(closePushButton);

        // add the layouts to the main layout

        mainLayout->addLayout(thermalLabelLayout);
        mainLayout->addLayout(thermalValueLayout);


        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("ui_loadsWindow", "Loads", Q_NULLPTR));

        //thermal
        thermalLoadLabel->setText(QApplication::translate("ui_loadsWindow", "Thermal", Q_NULLPTR));
        thermalLoadsComboBox->clear();
        thermalLoadsComboBox->insertItems(0, QStringList()
                                          << QApplication::translate("ui_loadsWindow", "Temperature", Q_NULLPTR)
                                          << QApplication::translate("ui_loadsWindow", "Convection", Q_NULLPTR)
                                          << QApplication::translate("ui_loadsWindow", "Radiation", Q_NULLPTR)
                                          << QApplication::translate("ui_loadsWindow", "Heat Flux", Q_NULLPTR)
                                          << QApplication::translate("ui_loadsWindow", "Internal Q", Q_NULLPTR)
                                          );
        thermalInputFieldLabel1->setText(QApplication::translate("ui_loadsWindow", "__________________", Q_NULLPTR));
        thermalInputFieldLabel2->setText(QApplication::translate("ui_loadsWindow", "__________________", Q_NULLPTR));

        thermalLineEdit1->setText(QApplication::translate("ui_loadsWindow", "0", Q_NULLPTR));
        thermalLineEdit2->setText(QApplication::translate("ui_loadsWindow", "0", Q_NULLPTR));

        applyThermalLoads->setText(QApplication::translate("ui_loadsWindow", "Apply", Q_NULLPTR));


        closePushButton->setText(QApplication::translate("ui_loadsWindow", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

/*******************************************************/
class ThermalLoadsWindow : public QDialog, public Ui_thermalLoadWindow {
    Q_OBJECT

public:
    ThermalLoadsWindow( QWidget *parent = 0);
    ~ThermalLoadsWindow();
void setViewer (TemplateViewer *v) { viewer = v; }

signals:
    void setThermalLoadsComplete();
private slots:
    void assignThermalLoads();
    void applyThermalLoad();
    void showEvent(QShowEvent *e);

  void closeDialog();

private:
   TemplateViewer *viewer;
   map<string,int>thermalLoadsMap;
    void makeConnections();
};

/*******************************************************/

