/********************************************************************************
** Form generated from reading UI file 'ui_geomWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_LATTICE_ANALYSIS_H
#define UI_UI_LATTICE_ANALYSIS_H

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

class Ui_LatticeAnalysisDialog
{
public:


    QVBoxLayout *mainLayout;

    QHBoxLayout *beamRadiusCutoffLayout;
    QLabel *beamRadiusCutoffLabel;
    QDoubleSpinBox *beamRadiusCutoffSpinBox;

    QPushButton *readTetMeshNodalStressPushButton;
    QPushButton *transferLoadsPushButton;
    QPushButton *staticAnalysisPushButton;
    QPushButton *modalAnalysisPushButton;
    QPushButton *optimizePushButton;
    QPushButton *removeThinBeamsPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *StaticAnalysisDialog)
    {
        if (StaticAnalysisDialog->objectName().isEmpty())
            StaticAnalysisDialog->setObjectName(QStringLiteral("StaticAnalysisDialog"));

        StaticAnalysisDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(StaticAnalysisDialog);

        readTetMeshNodalStressPushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(readTetMeshNodalStressPushButton);

        readTetMeshNodalStressPushButton->hide();

        transferLoadsPushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(transferLoadsPushButton);

        staticAnalysisPushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(staticAnalysisPushButton);

        modalAnalysisPushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(modalAnalysisPushButton);

        optimizePushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(optimizePushButton);

        beamRadiusCutoffLayout = new QHBoxLayout(StaticAnalysisDialog);
        beamRadiusCutoffLabel = new QLabel;
        beamRadiusCutoffSpinBox = new QDoubleSpinBox;
        beamRadiusCutoffSpinBox->setRange(0.0001, 1);
        beamRadiusCutoffSpinBox->setSingleStep(0.025);
        beamRadiusCutoffSpinBox->setValue(0.1);
        beamRadiusCutoffLayout->addWidget(beamRadiusCutoffLabel);
        beamRadiusCutoffLayout->addWidget(beamRadiusCutoffSpinBox);

        mainLayout->addLayout(beamRadiusCutoffLayout);

        removeThinBeamsPushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(removeThinBeamsPushButton);

        closePushButton = new QPushButton(StaticAnalysisDialog);
        mainLayout->addWidget(closePushButton);

        retranslateUi(StaticAnalysisDialog);

        QMetaObject::connectSlotsByName(StaticAnalysisDialog);
    } // setupUi

    void retranslateUi(QDialog *StaticAnalysisDialog)
    {
        StaticAnalysisDialog->setWindowTitle(QApplication::translate("StaticAnalysisDialog", "Lattice Analysis", Q_NULLPTR));
        readTetMeshNodalStressPushButton->setText(QApplication::translate("StaticAnalysisDialog", "Read TetMesh Results", Q_NULLPTR));
        transferLoadsPushButton->setText(QApplication::translate("StaticAnalysisDialog", "Transfer Structural Loads", Q_NULLPTR));
        staticAnalysisPushButton->setText(QApplication::translate("StaticAnalysisDialog", "Static FEA", Q_NULLPTR));
        modalAnalysisPushButton->setText(QApplication::translate("StaticAnalysisDialog", "Modal FEA", Q_NULLPTR));
        optimizePushButton->setText(QApplication::translate("StaticAnalysisDialog", "Optimize Frame", Q_NULLPTR));
        beamRadiusCutoffLabel->setText(QApplication::translate("StaticAnalysisDialog", "ThinBeam Cutoff Ratio:", Q_NULLPTR));
        removeThinBeamsPushButton->setText(QApplication::translate("StaticAnalysisDialog", "Remove thin beams", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("StaticAnalysisDialog", "Close", Q_NULLPTR));

    } // retranslateUi

};

QT_END_NAMESPACE


class LatticeAnalysisDialog : public QDialog, public Ui_LatticeAnalysisDialog {
    Q_OBJECT

public:
    LatticeAnalysisDialog( QWidget *parent = 0);
    ~LatticeAnalysisDialog();

    void setViewer(TemplateViewer *v) { viewer = v; }
signals:

private slots:
    void latticeFrameStaticFEA();
    void latticeFrameModalFEA();
    void readTetMeshFEAResultAndResizeBeams();
    void transferStructuralLoads();
    void suppressThinBeams();
    void latticeFrameOptimization();
    void closeDialog();
private:
    TemplateViewer *viewer;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////


#endif
