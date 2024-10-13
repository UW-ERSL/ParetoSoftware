#include "ui_structuralAnalysisDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include "CoreFunctions.hpp"

#define STATIC_STRUCTURAL_FEA 0
#define STATIC_THERMAL_FEA 1
#define MODAL_STRUCTURAL_FEA 2
#define TRANSIENT_THERMAL_FEA 3

using namespace QtConcurrent;
extern CoreFunctions           coreFunctions;
/////////////////////////////////////////////////////
StructuralAnalysisDialog::StructuralAnalysisDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();

    meshElementsMap[0] = 10000;
    meshElementsMap[1] = 25000;
    meshElementsMap[2] = 50000;
    meshElementsMap[3] = 100000;
    meshElementsMap[4] = 250000;

}
/////////////////////////////////////////////////////
StructuralAnalysisDialog::~StructuralAnalysisDialog()
{

}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::showEvent(QShowEvent *e)
{
    //first store values before updating, else liveVar gets updated
    int nelems;
    string tempUnitsName;

    coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,tempUnitsName);

    referenceTemperatureLabel->setText("Zero-strain T( " + QString::fromStdString(tempUnitsName)+ "): ");
    bool includeThermalEffects = liveVar.includeThermalEffects;
    bool remesh = liveVar.forceRemesh;
    bool meshApplied = liveVar.MESH_APPLIED;
    bool nodeBased = liveVar.nodeBasedKU;

    liveVar.currentBCSet = loadSpinBox->value();
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    liveVar.plotTopOptConstraints = true;

    nelems = liveVar.nElemsToMesh;
    numElemsSpinBox->setValue(nelems);


    includeThermalEffectCheckBox->setChecked(includeThermalEffects);
    remeshCheckBox->setChecked(remesh);
    loadSpinBox->setRange(0,liveVar.nBCSet-1);
    elemQualityCombo->setCurrentIndex(0);
    for (int i = 1; i < 5; i++)
        if (nelems >= meshElementsMap[i])
            elemQualityCombo->setCurrentIndex(i);

    nodeBasedCheckBox->setChecked(nodeBased);

    liveVar.MESH_APPLIED = meshApplied;// save the previous value
    liveVar.nodeBasedKU = nodeBased;
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::updateNumberOfElements(void)
{
    int quality = elemQualityCombo->currentIndex();
    numElemsSpinBox->setValue(meshElementsMap[quality]);
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::remeshCallBack(void)
{
    liveVar.forceRemesh = (int)remeshCheckBox->isChecked();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::updateLoadSetCallBack(void)
{
   liveVar.currentBCSet = loadSpinBox->value();
   liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
   liveVar.displayField = DISPLAY_FIELD_NONE;
   liveVar.plotElasticityBC = true;
   viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::updateLiveVariables(void)
{
    liveVar.MESH_APPLIED = false;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.nElemsToMesh = numElemsSpinBox->value();

    liveVar.workSpaceMethod = deflationTypeCombo->currentIndex();
    liveVar.nWorkSpaceGroups = numDeflationGroupsSpinBox->value();

    //liveVar.nWorkSpaceGroups = coreFunctions.getRecommendedDeflationGroups(liveVar.nElemsToMesh);
    liveVar.includeThermalEffects = includeThermalEffectCheckBox->isChecked();
    liveVar.useGPU = (int)GPUCheckBox->isChecked();
    liveVar.usePreconditioner = (int)preconditionerCheckBox->isChecked();
    liveVar.directSolver = (int)directSolverCheckBox->isChecked();
    liveVar.nodeBasedKU = (int)nodeBasedCheckBox->isChecked();

    liveVar.nEigenModes = (int) numModesSpinBox->value();
    #ifdef COMPILE_GPUCODE
    GPUCheckBox->show();
    #else
    GPUCheckBox->hide();
    #endif

    referenceTemperatureSpinBox->setEnabled(false);
    if (liveVar.includeThermalEffects)
        referenceTemperatureSpinBox->setEnabled(true);

    if(PARETO_RESEARCH)
    {
        preconditionerCheckBox->show();
        directSolverCheckBox->show();
        nodeBasedCheckBox->show();
    }
    else
    {
        preconditionerCheckBox->hide();
        directSolverCheckBox->hide();
        nodeBasedCheckBox->hide();
    }
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    liveVar.plotTopOptConstraints = true;

    vox.set_ThinStructure(false);
    if (thinStructureCheckBox->isChecked())
        vox.set_ThinStructure(true);
    viewer->updateGL();
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void runFEA(int feaType)
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;


    switch(feaType) {
    case STATIC_STRUCTURAL_FEA:
        coreFunctions.solveStatic();
        break;
    case STATIC_THERMAL_FEA:
        coreFunctions.solveThermoStatic();
        break;
    case TRANSIENT_THERMAL_FEA:
        coreFunctions.solveThermoTransient();
        break;
    case MODAL_STRUCTURAL_FEA:
        coreFunctions.solveModal();
        break;
    default:
        writeWarningToGUIWindow("Unknown FEA type");
        break;
    }
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::onFEAFinished()
{
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveThermalStatic()
{
    liveVar.nElemsToMesh = numElemsSpinBox->value();
    runFEA(STATIC_THERMAL_FEA);
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveStructuralStatic()
{
    liveVar.zeroStrainTemperature = referenceTemperatureSpinBox->value();
    liveVar.nElemsToMesh = numElemsSpinBox->value();
    runFEA(STATIC_STRUCTURAL_FEA);
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveStructuralModal()
{
    liveVar.nElemsToMesh = numElemsSpinBox->value();
    runFEA(MODAL_STRUCTURAL_FEA);
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::closeStructuralAnalysisDialog()
{
    emit setFeaComplete();
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void StructuralAnalysisDialog::makeConnections()
{
    SpinBoxi( loadSpinBox, [=]{ updateLoadSetCallBack();});
    ComboBox( elemQualityCombo, [=]{ remeshCheckBox->setChecked(true); updateNumberOfElements();});
    SpinBoxi( numElemsSpinBox, [=]{ updateLiveVariables();});
    ComboBox( deflationTypeCombo, [=]{ updateLiveVariables();});
    SpinBoxi( numDeflationGroupsSpinBox, [=]{ updateLiveVariables();});


    SpinBoxi( numModesSpinBox, [=]{ updateLiveVariables();});
    CheckBox( remeshCheckBox, [=]{ remeshCallBack();});
    CheckBox( includeThermalEffectCheckBox, [=]{updateLiveVariables();});
    CheckBox( preconditionerCheckBox, [=]{ updateLiveVariables();});
    CheckBox( thinStructureCheckBox, [=]{ updateLiveVariables();});
    CheckBox( GPUCheckBox, [=]{ remeshCheckBox->setChecked(true); updateLiveVariables();});
    CheckBox( directSolverCheckBox, [=]{ updateLiveVariables();});
    CheckBox( nodeBasedCheckBox, [=]{ updateLiveVariables();});

    PushButton( staticThermalSolvePushButton, [=]{ solveThermalStatic();});
    PushButton( staticStructuralSolvePushButton, [=]{ solveStructuralStatic();});
    PushButton( modalStructuralSolvePushButton, [=]{ solveStructuralModal();});
}
/////////////////////////////////////////////////////
