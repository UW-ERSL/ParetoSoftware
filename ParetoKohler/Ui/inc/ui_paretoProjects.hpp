#pragma once
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QDialog"
#include "TopOptViewer.hpp"

/* include the application header file */
#include "ui_supportStructure.hpp"

QT_BEGIN_NAMESPACE
/****************************************************/
class Ui_ParetoApplications
{
public:

    QVBoxLayout *mainLayout;

    //define the buttons that take us to the necessary ui
    QPushButton *supportStructurePushButton;

    QPushButton *closePushButton;


    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));


        Dialog->setMaximumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(Dialog);
        /***********************************************/
        // define the new push buttons

        supportStructurePushButton = new QPushButton(Dialog);

        closePushButton = new QPushButton(Dialog);
        /***********************************************/
        //add the push buttons to the main layout

        mainLayout->addWidget(supportStructurePushButton);
        mainLayout->addWidget(closePushButton);
        /**********************************************/
        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    }
    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "ParetoApplications", Q_NULLPTR));


        /***********************************************/
        // set the text for the push buttons
        supportStructurePushButton->setText(QApplication::translate("Dialog", "Support Structure", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Dialog", "Close", Q_NULLPTR));


        /***********************************************/
    } // retranslateUi

};

QT_END_NAMESPACE
/****************************************************/
class ParetoProjects : public QDialog, public Ui_ParetoApplications {
    Q_OBJECT

public:
    ParetoProjects( QWidget *parent = 0);
    ~ParetoProjects();
    void setViewer(TopOptViewer *v) { viewer = v;}

private slots:
    /* add the functions to the slots  to call the constructor of the different classes*/

    void openSupportStructureDialog();
    void closeDialog();
private:
    /* add objects for the different projects in pareto*/

    Ui_SupportStructure *supportStructure = nullptr;
    TopOptViewer *viewer;

    void makeConnections();

};
/****************************************************/


