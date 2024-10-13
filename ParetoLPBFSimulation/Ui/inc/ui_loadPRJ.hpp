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
#include "ParetoLPBFViewer.hpp"
QT_BEGIN_NAMESPACE

class Ui_LoadPRJDialog
{
public:
    QVBoxLayout *mainLayout;
    QPushButton *loadPRJPushButton;
    QPushButton *savePRJPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(Dialog);

        loadPRJPushButton = new QPushButton(Dialog);
        mainLayout->addWidget(loadPRJPushButton);

        savePRJPushButton = new QPushButton(Dialog);
        mainLayout->addWidget(savePRJPushButton);

        closePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(closePushButton);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Projects", Q_NULLPTR));
        loadPRJPushButton->setText(QApplication::translate("Dialog", "Load Pareto Project", Q_NULLPTR));
        savePRJPushButton->setText(QApplication::translate("Dialog", "Save Pareto Project", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Dialog", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class ProjectDialog : public QDialog, public Ui_LoadPRJDialog {
    Q_OBJECT

public:
    ProjectDialog( QWidget *parent = 0);
    ~ProjectDialog();

    void setViewer(TemplateViewer *v) { viewer = v;}
signals:
    void setProjectLoadingComplete();

private slots:
    void loadProjectDialog();
    void saveProjectDialog();
    void closeProjectDialog();



private:
    TemplateViewer *viewer;

    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_LOADPRJ_H

