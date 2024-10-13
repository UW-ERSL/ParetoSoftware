/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include "qglviewer.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include "ui_controlDialog.hpp"
#include "ParetoLPBFViewer.hpp"

QT_BEGIN_NAMESPACE

class ui_mainWindow
{
public:

    QAction *actionTools;
    QAction *actionExit;
    QAction *actionControl;
    QAction *actionLogMessage;
    QAction *actionApplicationMenu;
    QWidget *centralwidget;
    TemplateViewer *viewer;
    QProgressBar *progressBar;
    QFrame *renderFrame;
    QFrame *dialogframe;
    QScrollArea *logScrollArea;
    QMenuBar *menubar;
    QMenu *menuTopOpt;
    QStatusBar *statusbar;
    QTextEdit *logMessageBox;
    QDockWidget *dockWidget;
    QDockWidget *dockWidget2;

    void setupUi(QMainWindow *TopOptMainWindow)
    {
        if (TopOptMainWindow->objectName().isEmpty())
            TopOptMainWindow->setObjectName(QStringLiteral("TopOptMainWindow"));


        QRect rec = QApplication::desktop()->screenGeometry();
        int height = 0.85*rec.height();
        int width = 0.65*rec.width();

        TopOptMainWindow->resize(width, height); // set the intial display size
        //TopOptMainWindow->setFixedSize(width,height);
        actionExit = new QAction(TopOptMainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));

        progressBar = new QProgressBar(TopOptMainWindow);
        progressBar->setRange(0,100);
        actionTools = new QAction(TopOptMainWindow);
        actionTools->setObjectName(QStringLiteral("Tools"));

        actionLogMessage= new QAction(TopOptMainWindow);
        actionLogMessage->setObjectName(QStringLiteral("Log"));


        actionApplicationMenu = new QAction(TopOptMainWindow);
        actionApplicationMenu->setObjectName(QStringLiteral("actionApplication"));

        actionControl = new QAction(TopOptMainWindow);
        actionControl->setObjectName(QStringLiteral("actionControl"));
        // update the height and width according to screen size
        height = TopOptMainWindow->height();
        width = TopOptMainWindow->width();

        centralwidget = new QWidget(TopOptMainWindow);
        viewer = new TemplateViewer(centralwidget);

        viewer->setGeometry(QRect(0, 0, 0*width, 0*height));
        TopOptMainWindow->setCentralWidget(centralwidget);
        logMessageBox = new QTextEdit();
        logMessageBox->setReadOnly(true);

        QScrollBar *sb = logMessageBox->verticalScrollBar();
        sb->setValue(sb->maximum());


        dockWidget = new QDockWidget("");
        dockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        TopOptMainWindow->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
        dockWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        dockWidget->setWidget(logMessageBox);


        dockWidget2 = new QDockWidget("");
        TopOptMainWindow->addDockWidget(Qt::RightDockWidgetArea, dockWidget2);
        dockWidget2->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        dockWidget2->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        dockWidget2->setFeatures(QDockWidget::NoDockWidgetFeatures);

        menubar = new QMenuBar(TopOptMainWindow);
        menubar->setGeometry(QRect(0, 0, 1025, 20));
        menuTopOpt = new QMenu(menubar);
        TopOptMainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(TopOptMainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));

        TopOptMainWindow->setStatusBar(statusbar);
        menubar->addAction(menuTopOpt->menuAction());
        menuTopOpt->addAction(actionControl);

        TopOptMainWindow->statusBar()->addPermanentWidget(progressBar, 0);
        TopOptMainWindow->statusBar()->showMessage(QString(" "));

        menuTopOpt->addAction(actionApplicationMenu);
        menuTopOpt->addAction(actionTools);
        menuTopOpt->addAction(actionLogMessage);

        menuTopOpt->addAction(actionExit);

        menubar->hide();
        menuTopOpt->hide();
        retranslateUi(TopOptMainWindow);

        QMetaObject::connectSlotsByName(TopOptMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *TopOptMainWindow)
    {
        TopOptMainWindow->setWindowTitle(QApplication::translate("TopOptMainWindow", "MainWindow", Q_NULLPTR));
        actionExit->setText(QApplication::translate("TopOptMainWindow", "Exit", Q_NULLPTR));
        actionTools->setText(QApplication::translate("TopOptMainWindow", "Tools", Q_NULLPTR));
        actionControl->setText(QApplication::translate("TopOptMainWindow", "ParetoGUI", Q_NULLPTR));
        actionLogMessage ->setText(QApplication::translate("TopOptMainWindow", "Log", Q_NULLPTR));
        actionApplicationMenu->setText(QApplication::translate("TopOptMainWindow", "ParetoApps", Q_NULLPTR));
        menuTopOpt->setTitle(QApplication::translate("TopOptMainWindow", "GUIs", Q_NULLPTR));

    } // retranslateUi

};

QT_END_NAMESPACE


class TopOptMainWindow  : public QMainWindow,  public ui_mainWindow {
    Q_OBJECT

public:
    explicit TopOptMainWindow( QWidget *parent = NULL );
    ~TopOptMainWindow();

private slots:
    void  openControlDialog();
    void closeMainWindow();
    void openLogMessageBox();
private:
    void resizeEvent( QResizeEvent *e);
    void closeEvent( QCloseEvent *e);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void draw();

private:
    ControlDialog *controlDialog = nullptr;
    void makeConnections();
};
#endif // UI_MAINWINDOW_H
