/********************************************************************************
** Form generated from reading UI file 'ui_loadPRJ.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_LOADPRJ_H
#define UI_UI_LOADPRJ_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QDialog>
#include "LatticeViewer.hpp"


QT_BEGIN_NAMESPACE

class Ui_SaveFilesDialog
{
public:
    QVBoxLayout *mainLayout;
    QPushButton *saveGraphFilePushButton;
    QPushButton *saveRadiusFilePushButton;
    QPushButton *saveSolidLatticePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(Dialog);

        saveGraphFilePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(saveGraphFilePushButton);

        saveRadiusFilePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(saveRadiusFilePushButton);


        saveSolidLatticePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(saveSolidLatticePushButton);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Projects", Q_NULLPTR));

        saveGraphFilePushButton->setText(QApplication::translate("Dialog", "Save Lattice Graph File (.off)", Q_NULLPTR));
        saveRadiusFilePushButton->setText(QApplication::translate("Dialog", "Save Lattice Radius File (.txt)", Q_NULLPTR));
        saveSolidLatticePushButton->setText(QApplication::translate("Dialog", "Save Solid Lattice File (.stl)", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class SaveFilesDialog : public QDialog, public Ui_SaveFilesDialog {
    Q_OBJECT

public:
    SaveFilesDialog( QWidget *parent = 0);
    ~SaveFilesDialog();

    void setViewer(TemplateViewer *v) { viewer = v;}

private slots:
    void saveLatticeGraphFileDialog();
    void saveRadiusFileDialog();
    void saveSolidLatticeFileDialog();


private:
    TemplateViewer *viewer;

    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_LOADPRJ_H

