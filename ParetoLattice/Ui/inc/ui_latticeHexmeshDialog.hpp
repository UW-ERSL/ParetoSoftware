/********************************************************************************
** Form generated from reading UI file 'ui_geomWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_LATTICE_HEXMESH_H
#define UI_UI_LATTICE_HEXMESH_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QDialog>
#include <QColorDialog>
#include "LatticeViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_LatticeHexmeshDialog
{
public:


    QVBoxLayout *mainLayout;

    QPushButton *createHexmeshPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *StaticAnalysisDialog)
    {
        if (StaticAnalysisDialog->objectName().isEmpty())
            StaticAnalysisDialog->setObjectName(QStringLiteral("StaticAnalysisDialog"));

        StaticAnalysisDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(StaticAnalysisDialog);

        createHexmeshPushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(createHexmeshPushButton);


        closePushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(closePushButton);

        retranslateUi(StaticAnalysisDialog);

        QMetaObject::connectSlotsByName(StaticAnalysisDialog);
    } // setupUi

    void retranslateUi(QDialog *StaticAnalysisDialog)
    {
        StaticAnalysisDialog->setWindowTitle(QApplication::translate("StaticAnalysisDialog", "Lattice Hexmesh", Q_NULLPTR));
        createHexmeshPushButton->setText(QApplication::translate("StaticAnalysisDialog", "Create Hexmesh", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("StaticAnalysisDialog", "Close", Q_NULLPTR));

    } // retranslateUi

};

QT_END_NAMESPACE


class LatticeHexmeshDialog : public QDialog, public Ui_LatticeHexmeshDialog {
    Q_OBJECT

public:
    LatticeHexmeshDialog( QWidget *parent = 0);
    ~LatticeHexmeshDialog();

    void setViewer(TemplateViewer *v) { viewer = v; }
signals:

private slots:
    void createHexmesh();
    void closeDialog();
private:
    TemplateViewer *viewer;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////


#endif
