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

class Ui_gCodeDialog
{
public:
    QVBoxLayout *mainLayout;
    QLabel *spinSpeedLabel;
    QSpinBox *simulationSpinBox;
    QPushButton *loadGCodePushButton;
    QPushButton *simulateGCodePushButton;
    QPushButton *pauseGCodePushButton;
    QPushButton *stopGCodePushButton;
    QPushButton *closePushButton;
    QHBoxLayout *(propertyLayout[1]);
    QLabel *(propertyLabel[1]);
    QLineEdit *(propertyLineEdit[1]);

    void setupUi(QDialog *GCodeDialog)
    {
        if (GCodeDialog->objectName().isEmpty())
            GCodeDialog->setObjectName(QStringLiteral("GCodeDialog"));

        GCodeDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(GCodeDialog);

        spinSpeedLabel = new QLabel;
        simulationSpinBox = new QSpinBox;
        simulationSpinBox->setRange(1, 10);
        simulationSpinBox->setSingleStep(1);
        simulationSpinBox->setValue(5);

        propertyLayout[0] = new QHBoxLayout(GCodeDialog);
        propertyLabel[0] = new QLabel(GCodeDialog);
        propertyLineEdit[0] = new QLineEdit(GCodeDialog);
        propertyLabel[0]->setMinimumWidth(150);
        propertyLineEdit[0]->setMaximumWidth(120);
        propertyLineEdit[0]->setText(QString::number(4));
        propertyLabel[0]->setText(QApplication::translate("GCodeDialog", "Layer height (mm)", Q_NULLPTR));
        propertyLayout[0]->addWidget(propertyLabel[0]);
        propertyLayout[0]->addWidget(propertyLineEdit[0]);
        mainLayout->addLayout(propertyLayout[0]);

        loadGCodePushButton = new QPushButton(GCodeDialog);
        mainLayout->addWidget(loadGCodePushButton);

        simulateGCodePushButton = new QPushButton(GCodeDialog);
        mainLayout->addWidget(simulateGCodePushButton);

        pauseGCodePushButton = new QPushButton(GCodeDialog);
        mainLayout->addWidget(pauseGCodePushButton);

        stopGCodePushButton = new QPushButton(GCodeDialog);
        mainLayout->addWidget(stopGCodePushButton);

        closePushButton = new QPushButton(GCodeDialog);
        mainLayout->addWidget(closePushButton);

        QMetaObject::connectSlotsByName(GCodeDialog);

        retranslateUi(GCodeDialog);
    } // setupUi

    void retranslateUi(QDialog *GCodeDialog)
    {
        GCodeDialog->setWindowTitle(QApplication::translate("GCodeDialog", "GCodeDialog", Q_NULLPTR));
        loadGCodePushButton->setText(QApplication::translate("GCodeDialog", "Load GCode", Q_NULLPTR));
        simulateGCodePushButton->setText(QApplication::translate("GCodeDialog", "Simulate GCode", Q_NULLPTR));
        pauseGCodePushButton->setText(QApplication::translate("GCodeDialog", "Pause Simulation", Q_NULLPTR));
        stopGCodePushButton->setText(QApplication::translate("GCodeDialog", "Stop Simulation", Q_NULLPTR));

        closePushButton->setText(QApplication::translate("GCodeDialog", "Close", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
class GCodeDialog : public QDialog, public Ui_gCodeDialog{
    Q_OBJECT

public:
    GCodeDialog( QWidget *parent = 0);
    ~GCodeDialog();
void setViewer(TemplateViewer *v) { viewer = v; }

signals:
    void gCodeComplete();
private slots:
    void loadGCode();
    void simulateGCode();
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

