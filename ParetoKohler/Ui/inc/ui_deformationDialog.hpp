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

class Ui_deformationDialog
{
public:
    QVBoxLayout *mainLayout;


    QPushButton *simulatePrintingPushButton;
    QPushButton *simulateDryingPushButton;
    QPushButton *stopPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *DeformationDialog)
    {
        if (DeformationDialog->objectName().isEmpty())
            DeformationDialog->setObjectName(QStringLiteral("SpringDialog"));

        DeformationDialog->setMinimumWidth(GUI_MIN_WIDTH);

        mainLayout = new QVBoxLayout(DeformationDialog);


        simulatePrintingPushButton = new QPushButton(DeformationDialog);
        mainLayout->addWidget(simulatePrintingPushButton);

        simulateDryingPushButton = new QPushButton(DeformationDialog);
        mainLayout->addWidget(simulateDryingPushButton);

        stopPushButton = new QPushButton(DeformationDialog);
        mainLayout->addWidget(stopPushButton);

        closePushButton = new QPushButton(DeformationDialog);
        mainLayout->addWidget(closePushButton);

        QMetaObject::connectSlotsByName(DeformationDialog);

        retranslateUi(DeformationDialog);
    } // setupUi

    void retranslateUi(QDialog *DeformationDialog)
    {
        DeformationDialog->setWindowTitle(QApplication::translate("DeformationDialog", "DeformationDialog", Q_NULLPTR));
        simulatePrintingPushButton->setText(QApplication::translate("DeformationDialog", "Deformation during  Printing", Q_NULLPTR));
        simulateDryingPushButton->setText(QApplication::translate("DeformationDialog", "Deformation during Drying", Q_NULLPTR));
        stopPushButton->setText(QApplication::translate("DeformationDialog", "Stop Simulation", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("DeformationDialog", "Close", Q_NULLPTR));
    } // retranslateUi

};
QT_END_NAMESPACE
////////////////////////////////////////////////////////////////////////////////
class DeformationDialog : public QDialog, public Ui_deformationDialog{
    Q_OBJECT

public:
    DeformationDialog( QWidget *parent = 0);
    ~DeformationDialog();
void setViewer(TemplateViewer *v) { viewer = v; }

signals:

private slots:
    void simulateDeformationDuringPrinting();
    void simulateDeformationDuringDrying();
    void stopSimulation();
    void closeDialog();

private:
    TemplateViewer *viewer;
    bool    continueSimulation;
    void    makeConnections();

};
////////////////////////////////////////////////////////////////////////////////

