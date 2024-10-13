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
#include "TemplateViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_GeometryDialog
{
public:
    QVBoxLayout *mainLayout;
    QPushButton *loadSTLPushButton;

    QPushButton *closePushButton;

    void setupUi(QDialog *GeometryDialog)
    {
        if (GeometryDialog->objectName().isEmpty())
            GeometryDialog->setObjectName(QStringLiteral("GeometryDialog"));

        GeometryDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(GeometryDialog);

        loadSTLPushButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(loadSTLPushButton);


        closePushButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(closePushButton);



        retranslateUi(GeometryDialog);

        QMetaObject::connectSlotsByName(GeometryDialog);
    } // setupUi

    void retranslateUi(QDialog *GeometryDialog)
    {
        GeometryDialog->setWindowTitle(QApplication::translate("GeometryDialog", "Geometry", Q_NULLPTR));
        loadSTLPushButton->setText(QApplication::translate("GeometryDialog", "Load geometry", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("GeometryDialog", "Close", Q_NULLPTR));

    } // retranslateUi

};

namespace Ui {
    class GeometryDialog: public Ui_GeometryDialog {};
} // namespace Ui

QT_END_NAMESPACE


class GeometryDialog : public QDialog, public Ui_GeometryDialog {
    Q_OBJECT

public:
    GeometryDialog( QWidget *parent = 0);
    ~GeometryDialog();

    void setViewer(TemplateViewer *v) { viewer = v; }
signals:
    void setGeometryComplete();
private slots:
     void loadSTL();

     void saveSTL();
     void updateSTL();
     void saveAsOff();
     void generateIsoSTL();
     void closeDialog();
private:
    TemplateViewer *viewer;
    void makeConnections();
    map<string,int> unitMap;
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_GEOMWINDOW_H
