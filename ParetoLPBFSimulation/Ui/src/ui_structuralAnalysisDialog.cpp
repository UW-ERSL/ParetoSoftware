#include "ui_structuralAnalysisDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include "CoreFunctions.hpp"

#define STATIC_STRUCTURAL_FEA 0
#define STATIC_THERMAL_FEA 1
#define MODAL_STRUCTURAL_FEA 2
#define TRANSIENT_THERMAL_FEA 3

using namespace QtConcurrent;
extern void  writeWarningToGUIWindow(const char[],...);
extern CoreFunctions           coreFunctions;

extern bool getYesNoResponse(const char format[],...);
extern void writeWarningToGUIWindow(const char format[],...);
extern void writeMessageToGUIWindow(const char format[],...);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);
extern void updateProgressBar(const char title[], int percentComplete);
extern void resetProgressBar(const char title[]);
extern void setStatusMessage(const char str[]);
extern void grabImage(const char *fileName);

/************************************************************/
bool thermalFEACallBackFunction(double fractionCompleted)
{
    //topopt.pushThermalSolutionToTessellation();
    if (int(fractionCompleted*100) % 10 == 0)
        updateProgressBar("Thermal FEA Progress", (int) 100*fractionCompleted);
    return true;
}

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
    cout << nonlinearModeCheckBox->isChecked() << endl;

    if (nonlinearModeCheckBox->isChecked())
        solveNonLinearThermalStatic();
    else
        solveLinearThermalStatic();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveThermalTransient()
{
    if (nonlinearModeCheckBox->isChecked())
        solveNonLinearThermalTransient();
    else
        solveLinearThermalTransient();
}

/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveLinearThermalStatic()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;


    liveVar.nElemsToMesh = numElemsSpinBox->value();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = false;
    liveVar.THERMAL_FEA_SOLVED = false;// avoids repeated display
    setStatusMessage("Solving ThermoStatic");
    writeMessageToGUIWindow("Attempting to solve static thermal problem ...");
    if (!liveVar.BC_THERMAL_APPLIED ){
        writeWarningToGUIWindow("Thermal BC is undefined; please see specify BC");
        return;
    }
    coreFunctions.changeTopOptBC();
    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED))
        coreFunctions.createTopOptMesh();

    if (!liveVar.MESH_APPLIED){
        writeWarningToGUIWindow("Mesh was not created");
        return;
    }
    writeMessageToGUIWindow("Starting ThermoStatic FEA");

    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));

    topopt.disableGPU();
    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    liveVar.nWorkSpaceGroups = coreFunctions.getRecommendedDeflationGroups(liveVar.nElemsToMesh);

    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();


    Timer timer;
    timer.start();
    topopt.solveThermalProblem();
    writeMessageToGUIWindow("Time taken = %0.3f (s)",timer.getElapsedTimeInSec());

    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_TEMPERATURE;

    liveVar.plotAxis = 0;
    writeMessageToGUIWindow("Solved thermal ...");
    writeMessageToGUIWindow("Pushing thermal solution to tessellation ... ");

    topopt.pushThermalSolutionToTessellation();
    resetProgressBar("Static FEA complete!");

    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveLinearThermalTransient()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;

    int nNodes;
    double initialTemperature,simulationTime,timeStepSize;

    setStatusMessage("Solving Thermal Transient");

    if (!liveVar.BC_THERMAL_APPLIED ){
        writeWarningToGUIWindow("Thermal BC is undefined; please see specify BC");
        return;
    }

    coreFunctions.changeTopOptBC();
    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED))
        coreFunctions.createTopOptMesh();

    if (!liveVar.MESH_APPLIED){
        writeWarningToGUIWindow("Mesh was not created");
        return;
    }
    writeMessageToGUIWindow("Starting Thermo Transient FEA");

    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));

    topopt.disableGPU();
    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();

    initialTemperature = 300;
    simulationTime = 100000.0;
    timeStepSize = 500.0;

    Timer timer;

    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_TEMPERATURE;
    liveVar.plotAxis = 0;
    writeMessageToGUIWindow("Pushing thermal solution to tessellation ... ");
    timer.start();
    topopt.solveTransientThermoElasticProblem( initialTemperature,NULL, timeStepSize, simulationTime,false,thermalFEACallBackFunction);
    writeMessageToGUIWindow("Time taken = %0.3f (s)",timer.getElapsedTimeInSec());
    topopt.pushThermalSolutionToTessellation();
    resetProgressBar("Thermal Transient Complete");
    setStatusMessage("Thermal Transient Complete");

    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveNonLinearThermalStatic()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;


    liveVar.nElemsToMesh = numElemsSpinBox->value();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = false;
    liveVar.THERMAL_FEA_SOLVED = false;// avoids repeated display
    setStatusMessage("Solving Nonlinear ThermoStatic");
    writeMessageToGUIWindow("Attempting to solve nonlinear static thermal problem ...");
    if (!liveVar.BC_THERMAL_APPLIED ){
        writeWarningToGUIWindow("Thermal BC is undefined; please see specify BC");
        return;
    }
    coreFunctions.changeTopOptBC();
    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED))
        coreFunctions.createTopOptMesh();

    if (!liveVar.MESH_APPLIED){
        writeWarningToGUIWindow("Mesh was not created");
        return;
    }
    writeMessageToGUIWindow("Starting Nonlinear ThermoStatic FEA");

    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));

    topopt.disableGPU();
    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    liveVar.nWorkSpaceGroups = coreFunctions.getRecommendedDeflationGroups(liveVar.nElemsToMesh);

    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();


    float k[3],c[3],alpha[3];

    // from https://www.engineeringtoolbox.com/stainless-steel-specific-heat-thermal-conductivity-vs-temperature-d_2225.html
    k[0] = 8;
    k[1] = 0.0221;
    k[2] = 0.0;

    //  K (conductivity) depends on temperature as follows
    /*
     *  K = k[0] + k[1]*(T) + k[2]*(T)^2
     * */

    // assume constant for now
    c[0] = 1000;// heat capacity
    c[1] = 0.0;
    c[2] = 0.0;

    topopt.setNonLinearThermalCoefficients(k, c);
    Timer timer;

    timer.start();
    topopt.solveThermalProblem();// solve linear first
    topopt.solveNLStaticThermalProblem(false,thermalFEACallBackFunction);

    writeMessageToGUIWindow("Time taken = %0.3f (s)",timer.getElapsedTimeInSec());

    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_TEMPERATURE;

    liveVar.plotAxis = 0;
    writeMessageToGUIWindow("Solved thermal ...");
    writeMessageToGUIWindow("Pushing thermal solution to tessellation ... ");

    topopt.pushThermalSolutionToTessellation();
    resetProgressBar("Nonlinear Static FEA complete!");

    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveNonLinearThermalTransient()
{
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;


    liveVar.nElemsToMesh = numElemsSpinBox->value();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = false;
    liveVar.THERMAL_FEA_SOLVED = false;// avoids repeated display
    setStatusMessage("Solving Nonlinear ThermoStatic");
    writeMessageToGUIWindow("Attempting to solve nonlinear static thermal problem ...");
    if (!liveVar.BC_THERMAL_APPLIED ){
        writeWarningToGUIWindow("Thermal BC is undefined; please see specify BC");
        return;
    }
    coreFunctions.changeTopOptBC();
    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED))
        coreFunctions.createTopOptMesh();

    if (!liveVar.MESH_APPLIED){
        writeWarningToGUIWindow("Mesh was not created");
        return;
    }
    writeMessageToGUIWindow("Starting Nonlinear ThermoStatic FEA");

    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));

    topopt.disableGPU();
    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    liveVar.nWorkSpaceGroups = coreFunctions.getRecommendedDeflationGroups(liveVar.nElemsToMesh);

    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();


    float k[3],c[3],alpha[3];

    // from https://www.engineeringtoolbox.com/stainless-steel-specific-heat-thermal-conductivity-vs-temperature-d_2225.html
    k[0] = 8;
    k[1] = 0.0221;
    k[2] = 0.0;

    //  K (conductivity) depends on temperature as follows
    /*
     *  K = k[0] + k[1]*(T) + k[2]*(T)^2
     * */


    // assume constant for now
    c[0] = 1000;// heat capacity
    c[1] = 0.0;
    c[2] = 0.0;
    double initialTemperature = 300;
    double simulationTime = 100000.0;
    double timeStepSize = 500.0;

    topopt.setNonLinearThermalCoefficients(k, c);
    Timer timer;
    timer.start();

    topopt.solveNLTransientThermalProblem(initialTemperature,timeStepSize,simulationTime, false,thermalFEACallBackFunction);


    writeMessageToGUIWindow("Time taken = %0.3f (s)",timer.getElapsedTimeInSec());

    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_TEMPERATURE;

    liveVar.plotAxis = 0;
    writeMessageToGUIWindow("Solved thermal ...");
    writeMessageToGUIWindow("Pushing thermal solution to tessellation ... ");

    topopt.pushThermalSolutionToTessellation();
    resetProgressBar("Nonlinear Static FEA complete!");

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
    PushButton( transientThermalSolvePushButton, [=]{ solveThermalTransient();});
    PushButton( staticStructuralSolvePushButton, [=]{ solveStructuralStatic();});
    PushButton( modalStructuralSolvePushButton, [=]{ solveStructuralModal();});
}
/////////////////////////////////////////////////////
