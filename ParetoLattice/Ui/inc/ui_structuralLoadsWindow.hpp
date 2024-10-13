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
#include "LatticeViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_structuralLoadsWindow
{
public:
    QHBoxLayout *mainLayout;
    QVBoxLayout *structuralValueLayout;
    QVBoxLayout *structuralLabelLayout;

    QLabel *loadLabel;
    QSpinBox *loadNumberSpinBox;

    QLabel *selectionLabel;
    QComboBox *selectionCombo;

    QLabel *structuralLoadLabel;

    QPushButton *applyStructuralLoads;

    QComboBox *StructuralLoadsComboBox;
    QLabel *structualInputFieldLabel1;
    QLabel *structualInputFieldLabel2;
    QLabel *structualInputFieldLabel3;
    QLineEdit *structuralLineEdit1;
    QLineEdit *structuralLineEdit2;
    QLineEdit *structuralLineEdit3;

    QPushButton *closePushButton;
    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("LoadsWindow"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);
        //layouts
        mainLayout = new QHBoxLayout(Dialog);
        structuralLabelLayout = new QVBoxLayout(Dialog);
        structuralValueLayout = new QVBoxLayout(Dialog);

        //Load number
        loadLabel = new QLabel(Dialog);
        structuralLabelLayout->addWidget(loadLabel);
        loadNumberSpinBox = new QSpinBox;
        loadNumberSpinBox->setRange(0, 10);
        loadNumberSpinBox->setSingleStep(1);
        loadNumberSpinBox->setValue(0);
        structuralValueLayout->addWidget(loadNumberSpinBox);

        //selection type
        selectionLabel = new QLabel(Dialog);
        structuralLabelLayout->addWidget(selectionLabel);
        selectionCombo = new QComboBox(Dialog);
        structuralValueLayout->addWidget(selectionCombo);

        // structural load
        structuralLoadLabel = new QLabel(Dialog);
        structuralLabelLayout->addWidget(structuralLoadLabel);
        StructuralLoadsComboBox = new QComboBox(Dialog);
        structuralValueLayout->addWidget(StructuralLoadsComboBox);


        structualInputFieldLabel1 = new QLabel(Dialog);
        structuralLabelLayout->addWidget(structualInputFieldLabel1);

        structuralLineEdit1 = new QLineEdit(Dialog);
        structuralValueLayout->addWidget(structuralLineEdit1);

        structualInputFieldLabel2 = new QLabel(Dialog);
        structuralLabelLayout->addWidget(structualInputFieldLabel2);

        structuralLineEdit2 = new QLineEdit(Dialog);
        structuralValueLayout->addWidget(structuralLineEdit2);

        structualInputFieldLabel3 = new QLabel(Dialog);
        structuralLabelLayout->addWidget(structualInputFieldLabel3);

        structuralLineEdit3 = new QLineEdit(Dialog);
        structuralValueLayout->addWidget(structuralLineEdit3);

        applyStructuralLoads = new QPushButton(Dialog);
        structuralLabelLayout->addWidget(applyStructuralLoads);

        closePushButton = new QPushButton(Dialog);
        structuralValueLayout->addWidget(closePushButton);

        // add the layouts to the main layout
        mainLayout->addLayout(structuralLabelLayout);
        mainLayout->addLayout(structuralValueLayout);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("ui_loadsWindow", "Structural Loads", Q_NULLPTR));

        //load number
        loadLabel->setText(QApplication::translate("FEADialog", "Load Set", Q_NULLPTR));


        //selection type
        selectionLabel->setText(QApplication::translate("ui_loadsWindow", "Selection", Q_NULLPTR));
        selectionCombo->clear();
        selectionCombo->insertItems(0, QStringList()
                                             << QApplication::translate("ui_loadsWindow", "Triangle", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Plane", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Coarse Cylinder", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Fine Cylinder", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Other", Q_NULLPTR)
                                             );

        //structural loads
        structuralLoadLabel->setText(QApplication::translate("ui_loadsWindow", "Load Type", Q_NULLPTR));
        StructuralLoadsComboBox->clear();
        StructuralLoadsComboBox->insertItems(0, QStringList()
                                             << QApplication::translate("ui_loadsWindow", "None", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "XYZFixed", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "XFixed", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "YFixed", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "ZFixed", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Force", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Normal Force", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Pressure", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Torque", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Sliding", Q_NULLPTR)
                                             << QApplication::translate("ui_loadsWindow", "Retain", Q_NULLPTR)
                                             );
        structuralLineEdit1->setText(QApplication::translate("ui_loadsWindow", "0", Q_NULLPTR));
        structuralLineEdit2->setText(QApplication::translate("ui_loadsWindow", "0", Q_NULLPTR));
        structuralLineEdit3->setText(QApplication::translate("ui_loadsWindow", "0", Q_NULLPTR));

        structualInputFieldLabel1->setText(QApplication::translate("ui_loadsWindow", "________________", Q_NULLPTR));
        structualInputFieldLabel2->setText(QApplication::translate("ui_loadsWindow", "________________", Q_NULLPTR));
        structualInputFieldLabel3->setText(QApplication::translate("ui_loadsWindow", "________________", Q_NULLPTR));

        applyStructuralLoads->setText(QApplication::translate("ui_loadsWindow", "Apply ", Q_NULLPTR));

        closePushButton->setText(QApplication::translate("ui_loadsWindow", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
class StructuralLoadsWindow : public QDialog, public Ui_structuralLoadsWindow{
    Q_OBJECT

public:
    StructuralLoadsWindow( QWidget *parent = 0);
    ~StructuralLoadsWindow();
void setViewer(TemplateViewer *v) { viewer = v;}

signals:
void setStructuralLoadsComplete();

private slots:
    void updateLoadNumber();
    void updateSelectionType();
    void closeWindow();
    void assignStructuralLoads();
    void applyStructuralLoad();

private:
    TemplateViewer *viewer;
    void makeConnections();
    virtual void showEvent(QShowEvent * event);

    map<string ,int> structuralLoadsMap;

};
////////////////////////////////////////////////////////////////////////////////

