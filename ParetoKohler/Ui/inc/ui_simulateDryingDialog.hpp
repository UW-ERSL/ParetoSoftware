#pragma once

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include<QtWidgets/QVBoxLayout>
#include<QtWidgets/QHBoxLayout>

#include "QDialog"
#include "TemplateViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_simulateDryingDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *(propertyLayout[3]);
    QLabel *(propertyLabel[3]);
    QLineEdit *(propertyLineEdit[3]);

    QPushButton *simulateDryingPushButton;
    QPushButton *stopPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *DryingDialog)
    {
        if (DryingDialog->objectName().isEmpty())
            DryingDialog->setObjectName(QStringLiteral("Drying"));

        DryingDialog->setMinimumWidth(GUI_MIN_WIDTH);

        mainLayout = new QVBoxLayout(DryingDialog);

        propertyLayout[0] = new QHBoxLayout(DryingDialog);
        propertyLabel[0] = new QLabel(DryingDialog);
        propertyLineEdit[0] = new QLineEdit(DryingDialog);
        propertyLabel[0]->setMinimumWidth(150);
        propertyLineEdit[0]->setMaximumWidth(120);
        propertyLineEdit[0]->setText(QString::number(1));
        propertyLabel[0]->setText(QApplication::translate("Drying", "h-Drying (*1e-5 m/s)", Q_NULLPTR));
        propertyLayout[0]->addWidget(propertyLabel[0]);
        propertyLayout[0]->addWidget(propertyLineEdit[0]);
        mainLayout->addLayout(propertyLayout[0]);

        propertyLayout[1] = new QHBoxLayout(DryingDialog);
        propertyLabel[1] = new QLabel(DryingDialog);
        propertyLineEdit[1] = new QLineEdit(DryingDialog);
        propertyLabel[1]->setMinimumWidth(150);
        propertyLineEdit[1]->setMaximumWidth(120);
        propertyLineEdit[1]->setText(QString::number(2));
        propertyLabel[1]->setText(QApplication::translate("Drying", "Drying time (hrs) ", Q_NULLPTR));
        propertyLayout[1]->addWidget(propertyLabel[1]);
        propertyLayout[1]->addWidget(propertyLineEdit[1]);
        mainLayout->addLayout(propertyLayout[1]);

        propertyLayout[2] = new QHBoxLayout(DryingDialog);
        propertyLabel[2] = new QLabel(DryingDialog);
        propertyLineEdit[2] = new QLineEdit(DryingDialog);
        propertyLabel[2]->setMinimumWidth(150);
        propertyLineEdit[2]->setMaximumWidth(120);
        propertyLineEdit[2]->setText(QString::number(36));
        propertyLabel[2]->setText(QApplication::translate("Drying", "Drying steps ", Q_NULLPTR));
        propertyLayout[2]->addWidget(propertyLabel[2]);
        propertyLayout[2]->addWidget(propertyLineEdit[2]);
        mainLayout->addLayout(propertyLayout[2]);

        simulateDryingPushButton = new QPushButton(DryingDialog);
        mainLayout->addWidget(simulateDryingPushButton);

        stopPushButton = new QPushButton(DryingDialog);
        mainLayout->addWidget(stopPushButton);

        closePushButton = new QPushButton(DryingDialog);
        mainLayout->addWidget(closePushButton);

        QMetaObject::connectSlotsByName(DryingDialog);

        retranslateUi(DryingDialog);
    } // setupUi

    void retranslateUi(QDialog *DryingDialog)
    {
        DryingDialog->setWindowTitle(QApplication::translate("Simulation", "Drying", Q_NULLPTR));
        simulateDryingPushButton->setText(QApplication::translate("Drying", "Simulate Drying", Q_NULLPTR));
        stopPushButton->setText(QApplication::translate("Drying", "Stop Simulation", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("Drying", "Close", Q_NULLPTR));
    } // retranslateUi

};
QT_END_NAMESPACE
////////////////////////////////////////////////////////////////////////////////
class DryingDialog : public QDialog, public Ui_simulateDryingDialog{
    Q_OBJECT

public:
    DryingDialog( QWidget *parent = 0);
    ~DryingDialog();
void setViewer(TemplateViewer *v) { viewer = v; }

signals:
    void setMoistureComplete();
private slots:
    void simulateDrying();
    void stopSimulation();
    void closeDialog();

private:
    TemplateViewer *viewer;
    bool    continueSimulation;
    void    makeConnections();

};
////////////////////////////////////////////////////////////////////////////////

