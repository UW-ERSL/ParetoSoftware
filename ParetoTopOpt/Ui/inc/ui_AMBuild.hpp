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
#include <QTableWidget>
#include "TopOptViewer.hpp"


QT_BEGIN_NAMESPACE
/****************************************************/
class Ui_AMBuild
{
public:

    QVBoxLayout *mainLayout;

    //QHBoxLayout *overhangAngleLayout;
    //QLineEdit *overhangAngleLineEdit;
    //QLabel *overhangAngleLabel;

    //QPushButton *identifyOverhangTrianglesPushButton;

    QPushButton *identifyBuildDirectionsPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(Dialog);

        //identifyOverhangTrianglesPushButton = new QPushButton(Dialog);

        identifyBuildDirectionsPushButton = new QPushButton(Dialog);
        closePushButton = new QPushButton(Dialog);

        //overhangAngleLayout = new QHBoxLayout(Dialog);
        //overhangAngleLabel = new QLabel(Dialog);
        //overhangAngleLineEdit = new QLineEdit(Dialog);
        //overhangAngleLayout->addWidget(overhangAngleLabel);
        //overhangAngleLayout->addWidget(overhangAngleLineEdit);


        //mainLayout->addLayout(overhangAngleLayout);
        //mainLayout->addWidget(identifyOverhangTrianglesPushButton);

        mainLayout->addWidget(identifyBuildDirectionsPushButton);
        mainLayout->addWidget(closePushButton);



        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "AM Build", Q_NULLPTR));

        //overhangAngleLabel->setText(QApplication::translate("Dialog", "overhang angle", Q_NULLPTR));
        //overhangAngleLineEdit->setText(QApplication::translate("Dialog", "0", Q_NULLPTR));
        //overhangAngleLineEdit->setValidator(new QDoubleValidator(overhangAngleLineEdit));
        //identifyOverhangTrianglesPushButton->setText(QApplication::translate("Dialog", "Identify overhang", Q_NULLPTR));
        identifyBuildDirectionsPushButton->setText(QApplication::translate("Dialog", "Find Build Directions", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Dialog", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE
/****************************************************/
class AMBuildDialog : public QDialog, public Ui_AMBuild {
    Q_OBJECT

public:
    AMBuildDialog( QWidget *parent = 0);
    ~AMBuildDialog();

void setViewer(TopOptViewer *v) { viewer = v;}

signals:
    void setAMBuildLoadingComplete();

private slots:
    void identifyOverhang();
    void identifyBuildDirections();
    void updateDisplay();
    void closeDialog();
    void cellSelected(int nRow, int nCol);

private:
    TopOptViewer *viewer;
    QTableWidget* m_pTableWidget;

    QStringList m_TableHeader;
    void makeConnections();

};
/****************************************************/
