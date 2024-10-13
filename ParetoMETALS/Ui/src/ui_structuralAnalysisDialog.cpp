#include "ui_structuralAnalysisDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include "CoreFunctions.hpp"

#define STATIC_STRUCTURAL_FEA 0
#define STATIC_THERMAL_FEA 1
#define MODAL_STRUCTURAL_FEA 2
#define TRANSIENT_THERMAL_FEA 3

extern void writeWarningToGUIWindow(const char format[],...);
extern void resetProgressBar(const char title[]);
extern void setStatusMessage(const char str[]);
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
void StructuralAnalysisDialog::onFEAFinished()
{
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveThermalStatic()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;

    liveVar.nElemsToMesh = numElemsSpinBox->value();
    coreFunctions.solveThermoStatic();
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveStructuralStatic()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;

    liveVar.zeroStrainTemperature = referenceTemperatureSpinBox->value();
    liveVar.nElemsToMesh = numElemsSpinBox->value();
    coreFunctions.solveStatic();
    viewer->updateGL();
}

/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveGradedMaterialStructuralFEA()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;

    liveVar.zeroStrainTemperature = referenceTemperatureSpinBox->value();
    liveVar.nElemsToMesh = numElemsSpinBox->value();

    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED ))
        if (!coreFunctions.createTopOptMesh())
            return;

    writeMessageToParetoLog("***************************************************");
    // This solves the static problem with #templates = #elems
    double Ke0[24][24],Ke[24][24],rnd;

    //topopt.enableHexmeshVerbose();
    //topopt.enableDeflationVerbose();

    topopt.forceMaxTemplates();// every element is a template
    topopt.disableNodalSpMV();// cannot use nodal SpMV
    topopt.createElementMatricesTemplates(true);// to create memory


    /* Algorithm
     * 1. Create N materials
     * 2. Create Km for these N materials
     * 3. Let each element be a random mixture of these materials
     * 4. Create Ke for each element, with max templates
     * 5. Solve (N = 1 is standard solve).
     */
    topopt.getKeOfTemplateElem(0,Ke0);

    for (int elem = 0; elem < topopt.getNumElems(); elem++){
        rnd = 0.9 + 1.0*rand()/(1.0*RAND_MAX);
        for (int i = 0; i < 24; i++)
            for (int j = i; j < 24; j++)
                Ke[i][j] = rnd*Ke0[i][j];
        for (int i = 0; i < 24; i++)
            for (int j = 0; j < i; j++)
                Ke[i][j] = Ke[j][i];
        topopt.setKeOfTemplateElem(elem,Ke);
    }
    topopt.disableDeflation();
    topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    topopt.InitializeDeflationForElasticity();
    if (!topopt.solveElasticityStatic() ){
        writeWarningToGUIWindow("Could not solve FEA; check for disconnected components, or try increasing mesh size");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        resetProgressBar("");
        return;
    }

    topopt.postprocess();
    double timeForOneFEA;
    Timer timer;
    timer.start();

    timeForOneFEA = timer.getElapsedTimeInSec();
    liveVar.timeForOneFEA = timeForOneFEA;

    writeMessageToGUIWindow("#CG Iterations = %d",topopt.getLastCGIteration());
    writeMessageToGUIWindow("Time taken = %.3f (sec) ",timer.getElapsedTimeInSec());

    writeMessageToGUIWindow("Max Displacement: %0.2e",topopt.getDeformationMax());
    writeMessageToGUIWindow("Max Von Mises: %0.2e",topopt.getMaxVonMisesStress());
    writeMessageToGUIWindow("Time taken = %.3f (sec)",timer.getElapsedTimeInSec());


    writeMessageToGUIWindow("Pushing elasticity solution to tessellation ... ");
    topopt.pushInitialStaticSolutionToTessellation();
    liveVar.STATIC_FEA_SOLVED = true;
    liveVar.whichLoadSetElasticityFEASolved = liveVar.currentBCSet;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_STRESS;

    resetProgressBar("Static FEA complete!");
    liveVar.plotAxis = true;
    setStatusMessage("Static FEA Complete");


    viewer->updateGL();
}

/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveStructuralModal()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;

    liveVar.nElemsToMesh = numElemsSpinBox->value();
    coreFunctions.solveModal();
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
    PushButton( gradedMaterialStructuralSolvePushButton, [=]{ solveGradedMaterialStructuralFEA();});
    PushButton( modalStructuralSolvePushButton, [=]{ solveStructuralModal();});
}
/////////////////////////////////////////////////////
