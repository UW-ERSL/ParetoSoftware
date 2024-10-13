/********************************************************************************
** Form generated from reading UI file 'ui_topologyOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_MESHPROJECTIONDIALOG_H
#define UI_UI_MESHPROJECTIONDIALOG_H

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

class Ui_meshProjectionDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *remeshDensityLayout;
    QLabel *remeshDensityLabel;
    QDoubleSpinBox *remeshDensitySpinBox;

    QHBoxLayout *projectionDistanceLayout;
    QLabel *projectionDistanceLabel;
    QDoubleSpinBox *projectionDistanceSpinBox;

    QPushButton *projectIsosurfacePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        mainLayout = new QVBoxLayout(Dialog);

        projectionDistanceLabel = new QLabel(Dialog);
        projectionDistanceSpinBox = new QDoubleSpinBox(Dialog);
        projectionDistanceSpinBox->setRange(0,10.0);
        projectionDistanceSpinBox->setSingleStep(0.1);
        projectionDistanceSpinBox->setValue(1.1);
        projectionDistanceLayout = new QHBoxLayout(Dialog);
        projectionDistanceLayout->addWidget(projectionDistanceLabel);
        projectionDistanceLayout->addWidget(projectionDistanceSpinBox);

        remeshDensityLabel = new QLabel(Dialog);
        remeshDensitySpinBox = new QDoubleSpinBox(Dialog);
        remeshDensitySpinBox->setRange(0.1,2.0);
        remeshDensitySpinBox->setSingleStep(0.1);
        remeshDensitySpinBox->setValue(0.8);
        remeshDensityLayout = new QHBoxLayout(Dialog);
        remeshDensityLayout->addWidget(remeshDensityLabel);
        remeshDensityLayout->addWidget(remeshDensitySpinBox);

        projectIsosurfacePushButton = new QPushButton(Dialog);

        mainLayout->addLayout(projectionDistanceLayout);
        mainLayout->addLayout(remeshDensityLayout);
        mainLayout->addWidget(projectIsosurfacePushButton);


        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("meshProjectionDialog", "MeshProjection", Q_NULLPTR));

        projectionDistanceLabel->setText(QApplication::translate("meshProjectionDialog", "Projection Distance:", Q_NULLPTR));
        remeshDensityLabel->setText(QApplication::translate("meshProjectionDialog", "Remesh Density:", Q_NULLPTR));
        projectIsosurfacePushButton->setText(QApplication::translate("meshProjectionDialog", "Project Isosurface", Q_NULLPTR));

    } // retranslateUi

};

QT_END_NAMESPACE

class MeshProjectionDialog : public QDialog, public Ui_meshProjectionDialog {
    Q_OBJECT

public:
    MeshProjectionDialog( QWidget *parent = 0);
    ~MeshProjectionDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:


private slots:
    void projectIsosurface();
    void closeDialog();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif
