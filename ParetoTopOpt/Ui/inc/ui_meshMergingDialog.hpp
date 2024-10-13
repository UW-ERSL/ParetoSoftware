/********************************************************************************
** Form generated from reading UI file 'ui_topologyOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_MESHMERGINGDIALOG_H
#define UI_UI_MESHMERGINGDIALOG_H

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

class Ui_meshMergingDialog
{
public:
    QVBoxLayout *mainLayout;


    // CAD Control
    QHBoxLayout *CADControlLayout;
    QLabel *CADControlLabel;
    QDoubleSpinBox *CADControlSpinBox;


    // Isosurface Control
    QHBoxLayout *IsosurfaceControlLayout;
    QLabel *IsosurfaceControlLabel;
    QDoubleSpinBox *IsosurfaceControlSpinBox;


    QPushButton *mergeMeshesPushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        mainLayout = new QVBoxLayout(Dialog);

        CADControlLabel = new QLabel(Dialog);
        CADControlSpinBox = new QDoubleSpinBox(Dialog);
        CADControlSpinBox->setRange(0,10.0);
        CADControlSpinBox->setSingleStep(0.1);
        CADControlSpinBox->setValue(0.25);
        CADControlLayout = new QHBoxLayout(Dialog);
        CADControlLayout->addWidget(CADControlLabel);
        CADControlLayout->addWidget(CADControlSpinBox);

        IsosurfaceControlLabel = new QLabel(Dialog);
        IsosurfaceControlSpinBox = new QDoubleSpinBox(Dialog);
        IsosurfaceControlSpinBox->setRange(0,10.0);
        IsosurfaceControlSpinBox->setSingleStep(0.1);
        IsosurfaceControlSpinBox->setValue(0.25);
        IsosurfaceControlLayout = new QHBoxLayout(Dialog);
        IsosurfaceControlLayout->addWidget(IsosurfaceControlLabel);
        IsosurfaceControlLayout->addWidget(IsosurfaceControlSpinBox);

        mergeMeshesPushButton = new QPushButton(Dialog);


        mainLayout->addLayout(CADControlLayout);
        mainLayout->addLayout(IsosurfaceControlLayout);
        mainLayout->addWidget(mergeMeshesPushButton);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("meshMergingDialog", "Auto Design", Q_NULLPTR));

        CADControlLabel->setText(QApplication::translate("meshMergingDialog", "CAD Control:", Q_NULLPTR));
        IsosurfaceControlLabel->setText(QApplication::translate("meshMergingDialog", "Isosurf Control:", Q_NULLPTR));
        mergeMeshesPushButton->setText(QApplication::translate("meshMergingDialog", "Merge Mesh", Q_NULLPTR));


    } // retranslateUi

};

QT_END_NAMESPACE

class MeshMergingDialog : public QDialog, public Ui_meshMergingDialog {
    Q_OBJECT

public:
    MeshMergingDialog( QWidget *parent = 0);
    ~MeshMergingDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:


private slots:
    void mergeMesh();
    void closeDialog();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif
