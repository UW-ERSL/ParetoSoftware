/********************************************************************************
** Form generated from reading UI file 'ui_geomWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_GEOMWINDOW_H
#define UI_UI_GEOMWINDOW_H

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

class Ui_InputLatticeGraphDialog
{
public:

    QVBoxLayout *mainLayout;
    QPushButton *loadLatticeGraphPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *InputLatticeGraphDialog)
    {
        if (InputLatticeGraphDialog->objectName().isEmpty())
            InputLatticeGraphDialog->setObjectName(QStringLiteral("InputLatticeGraphDialog"));

        InputLatticeGraphDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(InputLatticeGraphDialog);

        loadLatticeGraphPushButton = new QPushButton(InputLatticeGraphDialog);
        mainLayout->addWidget(loadLatticeGraphPushButton);

        closePushButton = new QPushButton(InputLatticeGraphDialog);
        mainLayout->addWidget(closePushButton);
        closePushButton->hide();

        retranslateUi(InputLatticeGraphDialog);

        QMetaObject::connectSlotsByName(InputLatticeGraphDialog);
    } // setupUi

    void retranslateUi(QDialog *InputLatticeGraphDialog)
    {
        InputLatticeGraphDialog->setWindowTitle(QApplication::translate("InputLatticeGraphDialog", "Input", Q_NULLPTR));

        loadLatticeGraphPushButton->setText(QApplication::translate("InputLatticeGraphDialog", "Load Lattice Graph (.off)", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("InputLatticeGraphDialog", "Close", Q_NULLPTR));

    } // retranslateUi

};

namespace Ui {
    class InputLatticeGraphDialog: public Ui_InputLatticeGraphDialog {};
} // namespace Ui

QT_END_NAMESPACE


class InputLatticeGraphDialog : public QDialog, public Ui_InputLatticeGraphDialog {
    Q_OBJECT

public:
    InputLatticeGraphDialog( QWidget *parent = 0);
    ~InputLatticeGraphDialog();

    void setViewer(TemplateViewer *v) { viewer = v; }
signals:
    void setGeometryComplete();
private slots:
     void loadLatticeGraph();
     void closeDialog();
private:
    TemplateViewer *viewer;
    void makeConnections();
    map<string,int> unitMap;
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_GEOMWINDOW_H
