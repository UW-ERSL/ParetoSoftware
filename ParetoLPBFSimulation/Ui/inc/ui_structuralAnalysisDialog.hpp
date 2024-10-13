/********************************************************************************
** Form generated from reading UI file 'ui_StructuralAnalysisDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_StructuralAnalysisDialog_H
#define UI_UI_StructuralAnalysisDialog_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include "QDialog"
#include "ParetoLPBFViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_StructuralAnalysisDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *loadLayout;
    QHBoxLayout *qualityLayout;
    QHBoxLayout *elementsLayout;
    QHBoxLayout *deflationTypeLayout;
    QHBoxLayout *deflationNumGroupsLayout;
    QHBoxLayout *referenceTemperatureLayout;
    QHBoxLayout *modalLayout;

    QLabel *loadLabel;
    QSpinBox *loadSpinBox;

    QLabel *elemQualityLabel;
    QComboBox *elemQualityCombo;

    QLabel *numElemLabel;
    QSpinBox *numElemsSpinBox;

    QLabel *deflationTypeLabel;
    QComboBox *deflationTypeCombo;
    QLabel *deflationNumGroupsLabel;
    QSpinBox *numDeflationGroupsSpinBox;

    QLabel *numModesLabel;
    QSpinBox *numModesSpinBox;

    QCheckBox *includeThermalEffectCheckBox;

    QCheckBox *nonlinearModeCheckBox;

    QLabel *referenceTemperatureLabel; // for thermal-stress calculations
    QDoubleSpinBox *referenceTemperatureSpinBox;

    QCheckBox *thinStructureCheckBox;
    QCheckBox *remeshCheckBox;
    QCheckBox *preconditionerCheckBox;
    QCheckBox *GPUCheckBox;
    QCheckBox *directSolverCheckBox;
    QCheckBox *nodeBasedCheckBox;

    QPushButton *setMeshPushButton;
    QPushButton *staticThermalSolvePushButton;
    QPushButton *transientThermalSolvePushButton;
    QPushButton *staticStructuralSolvePushButton;
    QPushButton *modalStructuralSolvePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("AnalysisDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        // load
        loadLabel = new QLabel;
        loadSpinBox = new QSpinBox;
        loadSpinBox->setRange(0, 10);
        loadSpinBox->setSingleStep(1);
        loadSpinBox->setValue(0);

        // elem quality
        elemQualityLabel = new QLabel;
        elemQualityCombo = new QComboBox;

        // Number of elements
        numElemLabel = new QLabel;
        numElemsSpinBox = new QSpinBox;
        numElemsSpinBox->setRange(10, 100000000);
        numElemsSpinBox->setSingleStep(1000);
        numElemsSpinBox->setValue(10000);

        // Deflation
        deflationTypeLabel = new QLabel;
        deflationTypeCombo = new QComboBox;
        deflationNumGroupsLabel = new QLabel;
        numDeflationGroupsSpinBox =new QSpinBox;
        numDeflationGroupsSpinBox->setRange(1, 5000);
        numDeflationGroupsSpinBox->setSingleStep(10);
        numDeflationGroupsSpinBox->setValue(100);


        // Number of modes
        referenceTemperatureLabel = new QLabel;
        referenceTemperatureSpinBox = new QDoubleSpinBox;
        referenceTemperatureSpinBox->setRange(-1000, 10000);
        referenceTemperatureSpinBox->setSingleStep(10);
        referenceTemperatureSpinBox->setValue(300);

        numModesLabel = new QLabel;
        numModesSpinBox = new QSpinBox;
        numModesSpinBox->setRange(1, MAX_EIGEN_MODES);
        numModesSpinBox->setSingleStep(1);
        numModesSpinBox->setValue(1);

        includeThermalEffectCheckBox = new QCheckBox(Dialog);
        nonlinearModeCheckBox = new QCheckBox(Dialog);
        remeshCheckBox = new QCheckBox(Dialog);
        thinStructureCheckBox = new QCheckBox(Dialog);
        preconditionerCheckBox = new QCheckBox(Dialog);
        GPUCheckBox = new QCheckBox(Dialog);
        staticThermalSolvePushButton = new QPushButton(Dialog);
        transientThermalSolvePushButton = new QPushButton(Dialog);
        staticStructuralSolvePushButton = new QPushButton(Dialog);
        directSolverCheckBox = new QCheckBox(Dialog);
        directSolverCheckBox->setChecked(false);
        nodeBasedCheckBox = new QCheckBox(Dialog);
        modalStructuralSolvePushButton = new QPushButton(Dialog);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);
        loadLayout = new QHBoxLayout(Dialog);
        qualityLayout = new QHBoxLayout(Dialog);
        deflationTypeLayout = new QHBoxLayout(Dialog);
        deflationNumGroupsLayout = new QHBoxLayout(Dialog);
        elementsLayout = new QHBoxLayout(Dialog);
        modalLayout = new QHBoxLayout(Dialog);
        referenceTemperatureLayout = new QHBoxLayout(Dialog);

        //set mesh
        setMeshPushButton = new QPushButton(Dialog);
        loadLayout->addWidget(loadLabel);
        loadLayout->addWidget(loadSpinBox);

        qualityLayout->addWidget(elemQualityLabel);
        qualityLayout->addWidget(elemQualityCombo);

        elementsLayout->addWidget(numElemLabel);
        elementsLayout->addWidget(numElemsSpinBox);

        // deflation
        deflationTypeLayout->addWidget(deflationTypeLabel);
        deflationTypeLayout->addWidget(deflationTypeCombo);
        deflationNumGroupsLayout->addWidget(deflationNumGroupsLabel);
        deflationNumGroupsLayout->addWidget(numDeflationGroupsSpinBox);

        referenceTemperatureLayout->addWidget(referenceTemperatureLabel);
        referenceTemperatureLayout->addWidget(referenceTemperatureSpinBox);
        referenceTemperatureSpinBox->setEnabled(false);


        modalLayout->addWidget(numModesLabel);
        modalLayout->addWidget(numModesSpinBox);
        if (ONLY_MESH){
           // mainLayout->addLayout(deflationTypeLayout);
           // mainLayout->addLayout(deflationNumGroupsLayout);
            mainLayout->addLayout(modalLayout);
            mainLayout->addWidget(includeThermalEffectCheckBox);
            mainLayout->addWidget(nonlinearModeCheckBox);
            mainLayout->addLayout(referenceTemperatureLayout);
            mainLayout->addWidget(thinStructureCheckBox);
            mainLayout->addWidget(remeshCheckBox);
            mainLayout->addWidget(preconditionerCheckBox);
            mainLayout->addWidget(GPUCheckBox);
            mainLayout->addWidget(directSolverCheckBox);
            mainLayout->addWidget(nodeBasedCheckBox);
            mainLayout->addWidget(setMeshPushButton);
            mainLayout->addWidget(staticThermalSolvePushButton);
            mainLayout->addWidget(transientThermalSolvePushButton);
            mainLayout->addWidget(staticStructuralSolvePushButton);
            mainLayout->addWidget(modalStructuralSolvePushButton);
        }
        else {
            mainLayout->addLayout(loadLayout);
            mainLayout->addLayout(qualityLayout);
            mainLayout->addLayout(elementsLayout);
            //mainLayout->addLayout(deflationTypeLayout);
            //mainLayout->addLayout(deflationNumGroupsLayout);
            mainLayout->addLayout(modalLayout);
            mainLayout->addWidget(includeThermalEffectCheckBox);
            mainLayout->addWidget(nonlinearModeCheckBox);
            mainLayout->addLayout(referenceTemperatureLayout);
            mainLayout->addWidget(thinStructureCheckBox);
            mainLayout->addWidget(remeshCheckBox);
            mainLayout->addWidget(preconditionerCheckBox);
            mainLayout->addWidget(GPUCheckBox);
            mainLayout->addWidget(directSolverCheckBox);
            mainLayout->addWidget(nodeBasedCheckBox);
            mainLayout->addWidget(setMeshPushButton);
            mainLayout->addWidget(staticThermalSolvePushButton);
            mainLayout->addWidget(transientThermalSolvePushButton);
            mainLayout->addWidget(staticStructuralSolvePushButton);
            mainLayout->addWidget(modalStructuralSolvePushButton);
        }
        includeThermalEffectCheckBox->hide();
        staticStructuralSolvePushButton->hide();
        modalStructuralSolvePushButton->hide();


        // for compact mode, hide several buttons

#ifdef COMPILE_GPUCODE
GPUCheckBox->show();
#else
GPUCheckBox->hide();
#endif
        if (!PARETO_RESEARCH){
            //remeshCheckBox->hide();
            thinStructureCheckBox->hide();
            preconditionerCheckBox->hide();
            directSolverCheckBox->hide();
            nodeBasedCheckBox->hide();
        }
        setMeshPushButton->hide();
        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("AnalysisDialog", "FEA", Q_NULLPTR));

        elemQualityLabel->setText(QApplication::translate("AnalysisDialog", "Mesh Quality", Q_NULLPTR));
        elemQualityCombo->clear();
        elemQualityCombo->insertItems(0, QStringList()
                                      << QApplication::translate("AnalysisDialog", "Very Coarse", Q_NULLPTR)
                                      << QApplication::translate("AnalysisDialog", "Coarse", Q_NULLPTR)
                                      << QApplication::translate("AnalysisDialog", "Medium", Q_NULLPTR)
                                      << QApplication::translate("AnalysisDialog", "Fine", Q_NULLPTR)
                                      << QApplication::translate("AnalysisDialog", "Very Fine", Q_NULLPTR)
                                      );
        deflationTypeLabel->setText(QApplication::translate("AnalysisDialog", "Deflation Type", Q_NULLPTR));
        deflationTypeCombo->clear();
        deflationTypeCombo->insertItems(0, QStringList()
                                      << QApplication::translate("AnalysisDialog", "None", Q_NULLPTR)
                                      << QApplication::translate("AnalysisDialog", "Translate", Q_NULLPTR)
                                      << QApplication::translate("AnalysisDialog", "Rigid", Q_NULLPTR)
                                      << QApplication::translate("AnalysisDialog", "Strain", Q_NULLPTR)
                                      );
        deflationTypeCombo->setCurrentIndex(2);
        loadLabel->setText(QApplication::translate("AnalysisDialog", "Load Set", Q_NULLPTR));
        numElemLabel->setText(QApplication::translate("AnalysisDialog", "#Elements", Q_NULLPTR));
        deflationNumGroupsLabel->setText(QApplication::translate("AnalysisDialog", "#Deflation Groups", Q_NULLPTR));


        numModesLabel->setText(QApplication::translate("AnalysisDialog", "#Modes", Q_NULLPTR));

        includeThermalEffectCheckBox->setText(QApplication::translate("AnalysisDialog", "Include Thermal Effect", Q_NULLPTR));
        nonlinearModeCheckBox->setText(QApplication::translate("AnalysisDialog", "Nonlinear mode", Q_NULLPTR));
        referenceTemperatureLabel->setText(QApplication::translate("AnalysisDialog", "Zero-strain T(C): ", Q_NULLPTR));
        thinStructureCheckBox->setText(QApplication::translate("AnalysisDialog", "ThinStructure?", Q_NULLPTR));
        remeshCheckBox->setText(QApplication::translate("AnalysisDialog", "Remesh?", Q_NULLPTR));
        preconditionerCheckBox->setText(QApplication::translate("AnalysisDialog", "Preconditioner ? ", Q_NULLPTR));
        GPUCheckBox->setText(QApplication::translate("AnalysisDialog", "Use GPU? ", Q_NULLPTR));
        directSolverCheckBox->setText(QApplication::translate("AnalysisDialog", "Direct Solver? ", Q_NULLPTR));
        nodeBasedCheckBox->setText(QApplication::translate("AnalysisDialog", "Node Based? ", Q_NULLPTR));
        setMeshPushButton->setText(QApplication::translate("AnalysisDialog", "Mesh", Q_NULLPTR));
        staticThermalSolvePushButton->setText(QApplication::translate("AnalysisDialog", "Static Thermal Analysis", Q_NULLPTR));
        transientThermalSolvePushButton->setText(QApplication::translate("AnalysisDialog", "Transient Thermal Analysis", Q_NULLPTR));
        staticStructuralSolvePushButton->setText(QApplication::translate("AnalysisDialog", "Structural Analysis", Q_NULLPTR));
        modalStructuralSolvePushButton->setText(QApplication::translate("AnalysisDialog", "Modal Analysis", Q_NULLPTR));


    } // retranslateUi

};


QT_END_NAMESPACE

class StructuralAnalysisDialog : public QDialog, public Ui_StructuralAnalysisDialog {
    Q_OBJECT
public:
    StructuralAnalysisDialog( QWidget *parent = 0);
    ~StructuralAnalysisDialog();
    void setViewer(TemplateViewer *v) { viewer = v;}
signals:
    void setFeaComplete();

private slots:
    void solveThermalStatic();
    void solveLinearThermalStatic();
    void solveNonLinearThermalStatic();

    void solveThermalTransient();
    void solveLinearThermalTransient();
    void solveNonLinearThermalTransient();


    void solveStructuralStatic();


    void solveStructuralModal();


    void onFEAFinished();
    void showEvent(QShowEvent *e);
    void closeStructuralAnalysisDialog();
    void updateLoadSetCallBack(void);
    void remeshCallBack(void);
    void updateLiveVariables(void);
    void updateNumberOfElements(void);

private:
    TemplateViewer *viewer;
    void makeConnections();
    int meshElementsMap[5];
};
////////////////////////////////////////////////////////////////////////////////

#endif // UI_UI_StructuralAnalysisDialog_H
