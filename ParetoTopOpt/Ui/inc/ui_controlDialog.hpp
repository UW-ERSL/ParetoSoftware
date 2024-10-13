/********************************************************************************
** Form generated from reading UI file 'ui_ControlDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_CONTROLDIALOG_H
#define UI_UI_CONTROLDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TopOptViewer.hpp"
#include "ui_loadPRJ.hpp"
#include "ui_verboseDialog.hpp"
#include "ui_unitsDialog.hpp"
#include "ui_geomDialog.hpp"
#include "ui_materialDialog.hpp"
#include "ui_displayOptionsDialog.hpp"
#include "ui_ZernikeDialog.hpp"
#include "ui_structuralAnalysisDialog.hpp"
#include "ui_frequencyAnalysisDialog.hpp"
#include "ui_elastoPlasticDialog.hpp"
#include "ui_accelerationWindow.hpp"
#include "ui_constraintsDialog.hpp"
#include "ui_topologyOptimizationDialog.hpp"
#include "ui_autoDesignDialog.hpp"
#include "ui_meshMergingDialog.hpp"
#include "ui_meshProjectionDialog.hpp"
#include "ui_materialOptimizationDialog.hpp"
#include "ui_structuralLoadsWindow.hpp"
#include "ui_thermalLoadsWindow.hpp"
#include "ui_AMBuild.hpp"
#include "ui_optimizationResultDialog.hpp"
#include "ui_latticeStructureDialog.hpp"
#include "ui_thermalOptimizationDialog.hpp"

QT_BEGIN_NAMESPACE

class Ui_ControlDialog
{
public:

    QVBoxLayout *mainLayout;
    QPushButton *verbosePushButton;
    QPushButton *unitPushButton;
    QPushButton *geometryPushButton;
    QPushButton *ZernikePushButton;
    QPushButton *materialPushButton;
    QPushButton *accelerationPushButton;
    QPushButton *structuralLoadsPushButton;
    QPushButton *thermalLoadsPushButton;
    QPushButton *displayOptionsPushButton;
    QPushButton *StructuralAnalysisPushButton;
    QPushButton *FrequencyAnalysisPushButton;
    QPushButton *ElastoPlasticAnalysisPushButton;
    QPushButton *constraintsPushButton;
    QPushButton *optimizeTopologyPushButton;
    QPushButton *optimizeThermalPushButton;
    QPushButton *autoDesignPushButton;
    QPushButton *meshMergingPushButton;
    QPushButton *meshProjectionPushButton;
    QPushButton *optimizeMaterialPushButton;
    QPushButton *latticeStructurePushButton;
    QPushButton *AMBuildPushButton;
    QPushButton *resultPushButton;
    QPushButton *projectsPushButton;
    QPushButton *helpPushButton;

    void setupUi(QDialog *ControlDialog)
    {
        if (ControlDialog->objectName().isEmpty())
            ControlDialog->setObjectName(QStringLiteral("ControlDialog"));

        ControlDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(ControlDialog);

        verbosePushButton = new QPushButton(ControlDialog);
        verbosePushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(verbosePushButton);

        unitPushButton = new QPushButton(ControlDialog);
        unitPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(unitPushButton);

        geometryPushButton = new QPushButton(ControlDialog);
        geometryPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(geometryPushButton);

        ZernikePushButton = new QPushButton(ControlDialog);
        ZernikePushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(ZernikePushButton);

        materialPushButton = new QPushButton(ControlDialog);
        materialPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(materialPushButton);

        structuralLoadsPushButton = new QPushButton(ControlDialog);
        structuralLoadsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(structuralLoadsPushButton);

        thermalLoadsPushButton = new QPushButton(ControlDialog);
        thermalLoadsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(thermalLoadsPushButton);

        accelerationPushButton = new QPushButton(ControlDialog);
        accelerationPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(accelerationPushButton);

        displayOptionsPushButton = new QPushButton(ControlDialog);
        displayOptionsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(displayOptionsPushButton);

        StructuralAnalysisPushButton = new QPushButton(ControlDialog);
        StructuralAnalysisPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(StructuralAnalysisPushButton);

        FrequencyAnalysisPushButton = new QPushButton(ControlDialog);
        FrequencyAnalysisPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(FrequencyAnalysisPushButton);

        ElastoPlasticAnalysisPushButton = new QPushButton(ControlDialog);
        ElastoPlasticAnalysisPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(ElastoPlasticAnalysisPushButton);

        constraintsPushButton = new QPushButton(ControlDialog);
        constraintsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(constraintsPushButton);

        optimizeTopologyPushButton = new QPushButton(ControlDialog);
        optimizeTopologyPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(optimizeTopologyPushButton);

        optimizeThermalPushButton = new QPushButton(ControlDialog);
        optimizeThermalPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(optimizeThermalPushButton);

        autoDesignPushButton = new QPushButton(ControlDialog);
        autoDesignPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(autoDesignPushButton);

        meshMergingPushButton = new QPushButton(ControlDialog);
        meshMergingPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(meshMergingPushButton);



        meshProjectionPushButton = new QPushButton(ControlDialog);
        meshProjectionPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(meshProjectionPushButton);

        meshProjectionPushButton->hide();

        optimizeMaterialPushButton = new QPushButton(ControlDialog);
        optimizeMaterialPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(optimizeMaterialPushButton);

        resultPushButton = new QPushButton(ControlDialog);
        resultPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(resultPushButton);

        latticeStructurePushButton = new QPushButton(ControlDialog);
        latticeStructurePushButton->setStyleSheet("Text-align:left");
        //latticeStructurePushButton->hide();
        mainLayout->addWidget(latticeStructurePushButton);

        AMBuildPushButton = new QPushButton(ControlDialog);
        AMBuildPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(AMBuildPushButton);

        projectsPushButton = new QPushButton(ControlDialog);
        projectsPushButton->setStyleSheet("Text-align:left");
        mainLayout->addWidget(projectsPushButton);

        helpPushButton = new QPushButton(ControlDialog);
        mainLayout->addWidget(helpPushButton);

        if (!VERBOSE_GUI)
            verbosePushButton->hide();

        if (!AMBUILD_GUI){
            AMBuildPushButton->hide();
        }
        if (ONLY_FEA){
            constraintsPushButton->hide();
            optimizeThermalPushButton->hide();
            optimizeTopologyPushButton->hide();
            resultPushButton->hide();
        }
        if (ONLY_MESH){
            structuralLoadsPushButton->hide();
            thermalLoadsPushButton->hide();
        }
        if (!ZERNIKE_GUI){
            ZernikePushButton->hide();
        }
        if (!PARETO_RESEARCH){
            ElastoPlasticAnalysisPushButton->hide();
            meshMergingPushButton->hide();
            meshProjectionPushButton->hide();
            FrequencyAnalysisPushButton->hide();
        }
        if (!MATERIAL_OPTIMIZAION_GUI){
            optimizeMaterialPushButton->hide();
        }


        if (!LATTICE_GUI){
            latticeStructurePushButton->hide();
        }
        if (!AUTODESIGN_GUI){
            autoDesignPushButton->hide();
        }
        retranslateUi(ControlDialog);

        QMetaObject::connectSlotsByName(ControlDialog);
    } // setupUi

    void retranslateUi(QDialog *ControlDialog)
    {
        ControlDialog->setWindowTitle(QApplication::translate("ControlDialog", "Pareto", Q_NULLPTR));
        verbosePushButton->setText(QApplication::translate("ControlDialog", "Verbose", Q_NULLPTR));
        unitPushButton->setText(QApplication::translate("ControlDialog", "Units", Q_NULLPTR));
        geometryPushButton->setText(QApplication::translate("ControlDialog", "Geometry", Q_NULLPTR));
        ZernikePushButton->setText(QApplication::translate("ControlDialog", "Zernike", Q_NULLPTR));
        materialPushButton->setText(QApplication::translate("ControlDialog", "Material", Q_NULLPTR));
        accelerationPushButton->setText(QApplication::translate("ControlDialog", "Body force", Q_NULLPTR));
        structuralLoadsPushButton->setText(QApplication::translate("ControlDialog", "Structural Loads", Q_NULLPTR));
        thermalLoadsPushButton->setText(QApplication::translate("ControlDialog", "Thermal Loads", Q_NULLPTR));
        displayOptionsPushButton->setText(QApplication::translate("ControlDialog", "Display Options", Q_NULLPTR));
        StructuralAnalysisPushButton->setText(QApplication::translate("ControlDialog", "Analysis", Q_NULLPTR));
        FrequencyAnalysisPushButton->setText(QApplication::translate("ControlDialog", "Frequency Response", Q_NULLPTR));
        ElastoPlasticAnalysisPushButton->setText(QApplication::translate("ControlDialog", "Elasto-Plastic Analysis", Q_NULLPTR));
        constraintsPushButton->setText(QApplication::translate("ControlDialog", "TopOpt Constraints", Q_NULLPTR));
        optimizeTopologyPushButton->setText(QApplication::translate("ControlDialog", "Structural TopOpt", Q_NULLPTR));
        optimizeThermalPushButton->setText(QApplication::translate("ControlDialog", "Thermal TopOpt", Q_NULLPTR));
        autoDesignPushButton->setText(QApplication::translate("ControlDialog", "AutoDesign", Q_NULLPTR));
        meshMergingPushButton->setText(QApplication::translate("ControlDialog", "Mesh Merging", Q_NULLPTR));
        meshProjectionPushButton->setText(QApplication::translate("ControlDialog", "Mesh Projection", Q_NULLPTR));
        optimizeMaterialPushButton->setText(QApplication::translate("ControlDialog", "Material Optimization ", Q_NULLPTR));
        resultPushButton->setText(QApplication::translate("ControlDialog", "TopOpt Results", Q_NULLPTR));
        latticeStructurePushButton->setText(QApplication::translate("ControlDialog", "Lattice Design", Q_NULLPTR));
        AMBuildPushButton->setText(QApplication::translate("ControlDialog", "AM Build", Q_NULLPTR));
        projectsPushButton->setText(QApplication::translate("ControlDialog", "Projects", Q_NULLPTR));
        helpPushButton->setText(QApplication::translate("ControlDialog", "Help", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE

class ControlDialog : public QDialog, public Ui_ControlDialog {
    Q_OBJECT

public:
    ControlDialog( QWidget *parent = 0);
    ~ControlDialog();

    void setViewer(TopOptViewer *v) { viewer = v; }
private slots:
    void openVerboseDialog();
    void openUnitDialog();
    void openGeometryDialog();
    void openZernikeDialog();
    void openMaterialDialog();
    void openStructuralBoundCondDialog();
    void openThermalBoundCondDialog();
    void openAccelerationDialog();
    void openDisplayOptionsDialog();
    void openAMBuildDialog();
    void openStructuralAnalysisDialog();
    void openFrequencyAnalysisDialog();
    void openElastoPlasticAnalysisDialog();
    void openConstraintsDialog();
    void openOptimizeTopologyDialog();
    void openThermalOptimizationDialog();
    void openAutoDesignDialog();
    void openMeshMergingDialog();
    void openMeshProjectionDialog();
    void openOptimizeMaterialDialog();
    void openResultsDialog();
    void openLatticeStructureDialog();
    void openProjectsDialog();
    void closeControlDialog();
public slots:
    void updatePushButtons();

private:
    TopOptViewer *viewer;
    VerboseDialog  *verboseDialog = nullptr;
    UnitsDialog  *unitDialog = nullptr;
    GeometryDialog *geomDialog = nullptr;
    AMBuildDialog *amBuildDialog = nullptr;
    ProjectDialog *projectDialog = nullptr;
    MaterialDialog  *materialDialog = nullptr;
    ZernikeDialog  *zernikeDialog = nullptr;
    StructuralLoadsWindow  *structuralLoadsWindow = nullptr;
    ThermalLoadsWindow *thermalLoadsWindow = nullptr;
    DisplayOptionsDialog *displayOptionsDialog = nullptr;
    StructuralAnalysisDialog  *structuralAnalysisDialog = nullptr;
    FrequencyAnalysisDialog  *frequencyAnalysisDialog = nullptr;
    ElastoPlasticAnalysisDialog *elastoPlasticAnalysisDialog = nullptr;
    AccelerationDialog *accelDialog = nullptr;
    ConstraintsDialog *constraintsDialog = nullptr;
    TopologyOptimizationDialog *topOptDialog = nullptr;
    ThermalOptimizationDialog *thermalOptDialog = nullptr;
    AutoDesignDialog *autoDesignDialog = nullptr;
    MeshMergingDialog *meshMergingDialog = nullptr;
    MeshProjectionDialog *meshProjectionDialog = nullptr;
    MaterialOptimizationDialog *materialOptimizeDialog = nullptr;
    OptimizationResultDialog *optResultDialog = nullptr;
    LatticeStructureDialog *latticeStructureDialog = nullptr;

    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_CONTROLDIALOG_H
