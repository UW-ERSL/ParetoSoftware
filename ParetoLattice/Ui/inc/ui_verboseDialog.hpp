/********************************************************************************
** Form generated from reading UI file 'ui_feaDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_VERBOSEDIALOG_H
#define UI_UI_VERBOSEDIALOG_H

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
#include<QtWidgets/QVBoxLayout>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QSpinBox>
#include "QDialog"
#include "LatticeViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_VerboseDialog
{
public:
    QVBoxLayout *mainLayout;

    QCheckBox *STLVerboseCheckBox;
    QCheckBox *VoxVerboseCheckBox;
    QCheckBox *ElasticityVerboseCheckBox;
    QCheckBox *DeflationVerboseCheckBox;
    QCheckBox *ThermalVerboseCheckBox;
    QCheckBox *TopOptVerboseCheckBox;

    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("VerboseDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        STLVerboseCheckBox = new QCheckBox(Dialog);
        VoxVerboseCheckBox = new QCheckBox(Dialog);
        ElasticityVerboseCheckBox = new QCheckBox(Dialog);
        DeflationVerboseCheckBox = new QCheckBox(Dialog);
        ThermalVerboseCheckBox = new QCheckBox(Dialog);
        TopOptVerboseCheckBox = new QCheckBox(Dialog);
        closePushButton = new QPushButton(Dialog);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);
        mainLayout->addWidget(STLVerboseCheckBox);
        mainLayout->addWidget(VoxVerboseCheckBox);
        mainLayout->addWidget(ElasticityVerboseCheckBox);
        mainLayout->addWidget(DeflationVerboseCheckBox);
        mainLayout->addWidget(ThermalVerboseCheckBox);
        mainLayout->addWidget(TopOptVerboseCheckBox);
        mainLayout->addWidget(closePushButton);

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("VerboseDialog", "Verbose", Q_NULLPTR));
        STLVerboseCheckBox->setText(QApplication::translate("VerboseDialog", "STL Verbose", Q_NULLPTR));
        VoxVerboseCheckBox->setText(QApplication::translate("VerboseDialog", "Vox Verbose", Q_NULLPTR));
        ElasticityVerboseCheckBox->setText(QApplication::translate("VerboseDialog", "Elasticity Verbose", Q_NULLPTR));
        DeflationVerboseCheckBox->setText(QApplication::translate("VerboseDialog", "Deflation Verbose", Q_NULLPTR));
        ThermalVerboseCheckBox->setText(QApplication::translate("VerboseDialog", "Thermal Verbose", Q_NULLPTR));
        TopOptVerboseCheckBox->setText(QApplication::translate("VerboseDialog", "TopOpt Verbose", Q_NULLPTR));

        closePushButton->setText(QApplication::translate("VerboseDialog", "Close", Q_NULLPTR));
    } // retranslateUi

};


QT_END_NAMESPACE

class VerboseDialog : public QDialog, public Ui_VerboseDialog {
    Q_OBJECT
public:
    VerboseDialog( QWidget *parent = 0);
    ~VerboseDialog();
    void setViewer(TemplateViewer *v) { viewer = v;}
signals:
    ;

private slots:
    void showEvent(QShowEvent *e);
    void closeVerboseDialog();
    void updateLiveVariables(void);
private:
    TemplateViewer *viewer;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////

#endif
