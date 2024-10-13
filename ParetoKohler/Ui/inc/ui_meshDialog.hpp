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

class Ui_meshDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *(propertyLayout[4]);
    QLabel *(propertyLabel[4]);
    QLineEdit *(propertyLineEdit[4]);


    QPushButton *createMeshPushButton;
    QPushButton *simulateMeshPushButton;
    QPushButton *pausePushButton;
    QPushButton *stopPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *MeshDialog)
    {
        if (MeshDialog->objectName().isEmpty())
            MeshDialog->setObjectName(QStringLiteral("MeshDialog"));

        MeshDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(MeshDialog);

        propertyLayout[0] = new QHBoxLayout(MeshDialog);
        propertyLabel[0] = new QLabel(MeshDialog);
        propertyLineEdit[0] = new QLineEdit(MeshDialog);
        propertyLabel[0]->setMinimumWidth(150);
        propertyLineEdit[0]->setMaximumWidth(120);
        propertyLineEdit[0]->setText(QString::number(50));
        propertyLabel[0]->setText(QApplication::translate("MeshDialog", "Dry Clay E (MPa)", Q_NULLPTR));
        propertyLayout[0]->addWidget(propertyLabel[0]);
        propertyLayout[0]->addWidget(propertyLineEdit[0]);
        mainLayout->addLayout(propertyLayout[0]);

        propertyLayout[1] = new QHBoxLayout(MeshDialog);
        propertyLabel[1] = new QLabel(MeshDialog);
        propertyLineEdit[1] = new QLineEdit(MeshDialog);
        propertyLabel[1]->setMinimumWidth(150);
        propertyLineEdit[1]->setMaximumWidth(120);
        propertyLineEdit[1]->setText(QString::number(2200));
        propertyLabel[1]->setText(QApplication::translate("MeshDialog", "Dry Clay Density (kg/m^3)", Q_NULLPTR));
        propertyLayout[1]->addWidget(propertyLabel[1]);
        propertyLayout[1]->addWidget(propertyLineEdit[1]);
        mainLayout->addLayout(propertyLayout[1]);


        propertyLayout[2] = new QHBoxLayout(MeshDialog);
        propertyLabel[2] = new QLabel(MeshDialog);
        propertyLineEdit[2] = new QLineEdit(MeshDialog);
        propertyLabel[2]->setMinimumWidth(150);
        propertyLineEdit[2]->setMaximumWidth(120);
        propertyLineEdit[2]->setText(QString::number(20));
        propertyLabel[2]->setText(QApplication::translate("MeshDialog", "Clay Diffusivity (*1e-7 m^2/s)", Q_NULLPTR));
        propertyLayout[2]->addWidget(propertyLabel[2]);
        propertyLayout[2]->addWidget(propertyLineEdit[2]);
        mainLayout->addLayout(propertyLayout[2]);

        propertyLayout[3] = new QHBoxLayout(MeshDialog);
        propertyLabel[3] = new QLabel(MeshDialog);
        propertyLineEdit[3] = new QLineEdit(MeshDialog);
        propertyLabel[3]->setMinimumWidth(150);
        propertyLineEdit[3]->setMaximumWidth(120);
        propertyLineEdit[3]->setText(QString::number(50000));
        propertyLabel[3]->setText(QApplication::translate("MeshDialog", "Num of elements", Q_NULLPTR));
        propertyLayout[3]->addWidget(propertyLabel[3]);
        propertyLayout[3]->addWidget(propertyLineEdit[3]);
        mainLayout->addLayout(propertyLayout[3]);


        createMeshPushButton = new QPushButton(MeshDialog);
        mainLayout->addWidget(createMeshPushButton);

        simulateMeshPushButton = new QPushButton(MeshDialog);
        mainLayout->addWidget(simulateMeshPushButton);
        simulateMeshPushButton->hide();

        pausePushButton = new QPushButton(MeshDialog);
        mainLayout->addWidget(pausePushButton);
        pausePushButton->hide();

        stopPushButton = new QPushButton(MeshDialog);
        mainLayout->addWidget(stopPushButton);
        stopPushButton->hide();

        closePushButton = new QPushButton(MeshDialog);
        mainLayout->addWidget(closePushButton);
        closePushButton->hide();
        QMetaObject::connectSlotsByName(MeshDialog);

        retranslateUi(MeshDialog);
    } // setupUi

    void retranslateUi(QDialog *MeshDialog)
    {
        MeshDialog->setWindowTitle(QApplication::translate("MeshDialog", "MeshDialog", Q_NULLPTR));
        createMeshPushButton->setText(QApplication::translate("MeshDialog", "Create Mesh", Q_NULLPTR));
        simulateMeshPushButton->setText(QApplication::translate("MeshDialog", "Simulate Mesh", Q_NULLPTR));
        pausePushButton->setText(QApplication::translate("MeshDialog", "Pause Simulation", Q_NULLPTR));
        stopPushButton->setText(QApplication::translate("MeshDialog", "Stop Simulation", Q_NULLPTR));

        closePushButton->setText(QApplication::translate("MeshDialog", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
class MeshDialog : public QDialog, public Ui_meshDialog{
    Q_OBJECT

public:
    MeshDialog( QWidget *parent = 0);
    ~MeshDialog();
void setViewer(TemplateViewer *v) { viewer = v; }

signals:
    void setMeshComplete();
private slots:
    void createMesh();
    void simulateMesh();
    void pauseSimulation();
    void stopSimulation();
    void closeDialog();

private:
    TemplateViewer *viewer;
    bool    pausedSimulation;
    bool    continueSimulation;
    void makeConnections();

};
////////////////////////////////////////////////////////////////////////////////

