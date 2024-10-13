/********************************************************************************
** Form generated from reading UI file 'ui_ControlDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_CONTROLDIALOG_H
#define UI_UI_CONTROLDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TemplateViewer.hpp"
#include "ui_loadPRJ.hpp"
#include "ui_verboseDialog.hpp"
#include "ui_unitsDialog.hpp"
#include "ui_geomDialog.hpp"
#include "ui_materialDialog.hpp"
#include "ui_deformationDialog.hpp"
#include "ui_displayOptionsDialog.hpp"
#include "ui_gCodeDialog.hpp"
#include "ui_meshDialog.hpp"
#include "ui_simulatePrintingDialog.hpp"
#include "ui_simulateDryingDialog.hpp"

QT_BEGIN_NAMESPACE

class Ui_ControlDialog
{
public:

    QVBoxLayout *mainLayout;
    QPushButton *verbosePushButton;
    QPushButton *unitPushButton;
    QPushButton *geometryPushButton;
    QPushButton *materialPushButton;
    QPushButton *gCodePushButton;
    QPushButton *meshPushButton;
    QPushButton *printingPushButton;
    QPushButton *dryingPushButton;
    QPushButton *displayOptionsPushButton;
    QPushButton *projectsPushButton;
    QPushButton *helpPushButton;

    void setupUi(QDialog *ControlDialog)
    {
        if (ControlDialog->objectName().isEmpty())
            ControlDialog->setObjectName(QStringLiteral("ControlDialog"));

        ControlDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(ControlDialog);

        verbosePushButton = new QPushButton(ControlDialog);
        verbosePushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(verbosePushButton);
        verbosePushButton->hide();

        unitPushButton = new QPushButton(ControlDialog);
        unitPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(unitPushButton);
        unitPushButton->hide();

        geometryPushButton = new QPushButton(ControlDialog);
        geometryPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(geometryPushButton);

        materialPushButton = new QPushButton(ControlDialog);
        materialPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(materialPushButton);
        materialPushButton->hide();
        gCodePushButton = new QPushButton(ControlDialog);
        gCodePushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(gCodePushButton);

        meshPushButton = new QPushButton(ControlDialog);
        meshPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(meshPushButton);

        printingPushButton = new QPushButton(ControlDialog);
        printingPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(printingPushButton);

        dryingPushButton = new QPushButton(ControlDialog);
        dryingPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(dryingPushButton);

        displayOptionsPushButton = new QPushButton(ControlDialog);
        displayOptionsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(displayOptionsPushButton);


        projectsPushButton = new QPushButton(ControlDialog);
        projectsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(projectsPushButton);
        projectsPushButton->hide();

        helpPushButton = new QPushButton(ControlDialog);
        mainLayout->addWidget(helpPushButton);

        retranslateUi(ControlDialog);

        QMetaObject::connectSlotsByName(ControlDialog);
    } // setupUi

    void retranslateUi(QDialog *ControlDialog)
    {
        ControlDialog->setWindowTitle(QApplication::translate("ControlDialog", "Pareto", Q_NULLPTR));
        verbosePushButton->setText(QApplication::translate("ControlDialog", "Verbose", Q_NULLPTR));
        unitPushButton->setText(QApplication::translate("ControlDialog", "Units", Q_NULLPTR));
        geometryPushButton->setText(QApplication::translate("ControlDialog", "Geometry", Q_NULLPTR));
        materialPushButton->setText(QApplication::translate("ControlDialog", "Material", Q_NULLPTR));
        gCodePushButton->setText(QApplication::translate("ControlDialog", "G-Code", Q_NULLPTR));
        meshPushButton->setText(QApplication::translate("ControlDialog", "Material and Mesh", Q_NULLPTR));
        printingPushButton->setText(QApplication::translate("ControlDialog", "Simulate Printing", Q_NULLPTR));
        dryingPushButton->setText(QApplication::translate("ControlDialog", "Simulate Drying", Q_NULLPTR));
        displayOptionsPushButton->setText(QApplication::translate("ControlDialog", "Display Options", Q_NULLPTR));

        projectsPushButton->setText(QApplication::translate("ControlDialog", "Projects", Q_NULLPTR));
        helpPushButton->setText(QApplication::translate("ControlDialog", "Help", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class ControlDialog : public QDialog, public Ui_ControlDialog {
    Q_OBJECT

public:
    ControlDialog( QWidget *parent = 0);
    ~ControlDialog();

    void setViewer(TemplateViewer *v) { viewer = v; }
private slots:
    void openVerboseDialog();
    void openUnitDialog();
    void openGeometryDialog();
    void openMaterialDialog();
    void openGCodeDialog();
    void openMeshDialog();
    void openPrintingDialog();
    void openDryingDialog();
    void openDisplayOptionsDialog();
    void openProjectsDialog();
    void closeControlDialog();
public slots:
    void updatePushButtons();

private:
    TemplateViewer *viewer;
    VerboseDialog  *verboseDialog = nullptr;
    UnitsDialog  *unitDialog = nullptr;
    GeometryDialog *geomDialog = nullptr;
    MaterialDialog  *materialDialog = nullptr;
    GCodeDialog  *gCodeDialog = nullptr;
    MeshDialog  *meshDialog = nullptr;
    PrintingDialog  *printingDialog = nullptr;
    DryingDialog  *dryingDialog = nullptr;
    ProjectDialog *projectDialog = nullptr;

    DisplayOptionsDialog *displayOptionsDialog = nullptr;


    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_CONTROLDIALOG_H
