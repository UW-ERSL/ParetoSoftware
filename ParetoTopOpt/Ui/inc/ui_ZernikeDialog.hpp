/********************************************************************************
** Form generated from reading UI file 'ui_StructuralZernikeDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_ZernikeDialog_H
#define UI_UI_ZernikeDialog_H

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

class Ui_ZernikeDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *gridSizeLayout;
    QHBoxLayout *ZernikeOrderLayout;

    QLabel *gridSizeLabel;
    QSpinBox *gridSizeSpinBox;

    QLabel *ZernikeOrderLabel;
    QSpinBox *ZernikeOrderSpinBox;
    QPushButton *computeZernikePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("ZernikeDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        // Number of elements
        gridSizeLabel = new QLabel;
        gridSizeSpinBox = new QSpinBox;
        gridSizeSpinBox->setRange(4, 256);
        gridSizeSpinBox->setSingleStep(1);
        gridSizeSpinBox->setValue(64);

        ZernikeOrderLabel = new QLabel;
        ZernikeOrderSpinBox = new QSpinBox;
        ZernikeOrderSpinBox->setRange(1, 20);
        ZernikeOrderSpinBox->setSingleStep(1);
        ZernikeOrderSpinBox->setValue(6);

        computeZernikePushButton = new QPushButton(Dialog);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);

        gridSizeLayout = new QHBoxLayout(Dialog);
        ZernikeOrderLayout  = new QHBoxLayout(Dialog);

        gridSizeLayout->addWidget(gridSizeLabel);
        gridSizeLayout->addWidget(gridSizeSpinBox);

        ZernikeOrderLayout->addWidget(ZernikeOrderLabel);
        ZernikeOrderLayout->addWidget(ZernikeOrderSpinBox);

        mainLayout->addLayout(gridSizeLayout);
        mainLayout->addLayout(ZernikeOrderLayout);

        mainLayout->addWidget(computeZernikePushButton);
        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("ZernikeDialog", "Zernike", Q_NULLPTR));

        gridSizeLabel->setText(QApplication::translate("ZernikeDialog", "Grid Size:", Q_NULLPTR));
        ZernikeOrderLabel->setText(QApplication::translate("ZernikeDialog", "Zernike Order:", Q_NULLPTR));

        computeZernikePushButton->setText(QApplication::translate("ZernikeDialog", "Compute Zernike", Q_NULLPTR));
    } // retranslateUi

};


QT_END_NAMESPACE

class ZernikeDialog : public QDialog, public Ui_ZernikeDialog {
    Q_OBJECT
public:
    ZernikeDialog( QWidget *parent = 0);
    ~ZernikeDialog();
    void setViewer(TopOptViewer *v) { viewer = v;}

private slots:
    void showEvent(QShowEvent *e);
    void closeZernikeDialog();
    void computeZernikeCoefficients(void);
private:
    TopOptViewer *viewer;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////

#endif // UI_UI_ZernikeDialog_H
