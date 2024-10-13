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

#include "LatticeViewer.hpp"
#include "ui_saveFiles.hpp"
#include "ui_verboseDialog.hpp"
#include "ui_unitsDialog.hpp"
#include "ui_displayOptionsDialog.hpp"
#include "ui_latticeGraphDialog.hpp"
#include "ui_solidLatticeDialog.hpp"

QT_BEGIN_NAMESPACE

class Ui_ControlDialog
{
public:

    QVBoxLayout *mainLayout;
    QPushButton *verbosePushButton;
    QPushButton *unitPushButton;
    QPushButton *latticeCreateDialogPushButton;
    QPushButton *createSolidLatticePushButton;
    QPushButton *displayOptionsPushButton;
    QPushButton *saveFilePushButton;
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

        latticeCreateDialogPushButton = new QPushButton(ControlDialog);
        latticeCreateDialogPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(latticeCreateDialogPushButton);

        createSolidLatticePushButton = new QPushButton(ControlDialog);
        createSolidLatticePushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(createSolidLatticePushButton);

        displayOptionsPushButton = new QPushButton(ControlDialog);
        displayOptionsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(displayOptionsPushButton);

        saveFilePushButton = new QPushButton(ControlDialog);
        saveFilePushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(saveFilePushButton);

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
        latticeCreateDialogPushButton->setText(QApplication::translate("ControlDialog", "Load/Create Lattice Graph", Q_NULLPTR));
        createSolidLatticePushButton->setText(QApplication::translate("ControlDialog", "Create Solid Lattice", Q_NULLPTR));
        displayOptionsPushButton->setText(QApplication::translate("ControlDialog", "Display Options", Q_NULLPTR));
        saveFilePushButton->setText(QApplication::translate("ControlDialog", "Save Files", Q_NULLPTR));
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
    void openDisplayOptionsDialog();
    void openLatticeStructureDialog();
    void openCreateSolidLatticeDialog();
    void openSaveFilesDialog();
    void closeControlDialog();
public slots:
    void updatePushButtons();

private:
    TemplateViewer *viewer;
    VerboseDialog  *verboseDialog = nullptr;
    UnitsDialog  *unitDialog = nullptr;
    LatticeStructureDialog  *latticeStructureDialog = nullptr;
    LatticeRadiiDialog  *latticeRadiiDialog = nullptr;
    SaveFilesDialog *saveFilesDialog = nullptr;
    DisplayOptionsDialog *displayOptionsDialog = nullptr;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_CONTROLDIALOG_H
