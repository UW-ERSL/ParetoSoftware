#include "ui_structuralAnalysisDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include "CoreFunctions.hpp"
#define CD_NAMESPACE ChartDirNameSpace
#include "chartdir.h"

#define STATIC_STRUCTURAL_FEA 0
#define STATIC_THERMAL_FEA 1
#define MODAL_STRUCTURAL_FEA 2
#define TRANSIENT_THERMAL_FEA 3

extern void writeWarningToGUIWindow(const char format[],...);
using namespace QtConcurrent;
extern CoreFunctions           coreFunctions;

extern void resetProgressBar(const char title[]);
extern void updateProgressBar(const char title[], int percentComplete);

vector<double> forceData,displacementData;

bool structuralDynamicsCallBack(double fractionCompleted)
{
    topopt.pushInitialStaticSolutionToTessellation();
    liveVar.STATIC_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;

    updateProgressBar("Structural Dynamics Progress", (int) 100*fractionCompleted);

    return true;

}

bool nonlinearCallback(double fractionCompleted, double *reactionForces)
{
    int nNodes, zdof;
    double Fz, totalReaction,uvwMin[3],uvwMax[3],zDisp;

    nNodes = topopt.getNumNodes();
    totalReaction = 0;
    for (int i = 0; i < nNodes; i++){
        zdof = 3*i+2;
        Fz = reactionForces[zdof];
        if (Fz > 0){// upward reaction
            totalReaction += Fz;
        }
    }

    topopt.getUVWMinMax(uvwMin,uvwMax);
    zDisp = 1000*fabs(uvwMin[2]);
    writeMessageToParetoLog("*******Call back; Fz = %.2f (N) , zDisp = %.2f (mm) ",totalReaction,zDisp);
    forceData.push_back(totalReaction);
    displacementData.push_back(zDisp);// negative Z


    topopt.pushInitialStaticSolutionToTessellation();
    liveVar.STATIC_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;

    updateProgressBar("Nonlinear FEA Progress", (int) 100*fractionCompleted);

    return true;
}

void plotXY(vector<double> xData, vector<double> yData, bool smooth, char *xTitle, char *yTitle, char *chartTitle, int width = 350, int height = 300)
{
    int nXSamples;
    double *xValues, *yValues, yMax, yMin;

    nXSamples = (int)xData.size();
    if (yData.size() != nXSamples) {
        printf("Incompatible data \n");
        return;
    }
    xValues = (double *)malloc(nXSamples * sizeof(double));
    yValues = (double *)malloc(nXSamples * sizeof(double));
    for (int j = 0; j < nXSamples; j++) {
        xValues[j] = xData[j];
        yValues[j] = yData[j];

    }

    XYChart *c = new XYChart(width, height, 0xcce6ff, 0x000000, 1);
    c->setPlotArea(75, 40, width - 100, height - 80, 0xffffff, -1, -1, 0xc0c0c0, -1);
    c->addLegend(60, 15, false, "", 8)->setBackground(Chart::Transparent);
    c->addTitle(chartTitle, "arialbd.ttf", 12, 0x000000);

    yMax = *max_element(yData.begin(), yData.end());
    yMin = *min_element(yData.begin(), yData.end());

    /*
    if ((fabs(yMax) > 0.1) && (fabs(yMax) < 1000) && (fabs(yMin) > 0.1) && (fabs(yMin) < 1000))
        c->yAxis()->setLabelFormat("{value|P4}");// precision
    else
        c->yAxis()->setLabelFormat("{value|E2}");
    */

    c->xAxis()->setLabelStep(nXSamples / 10.0);
    c->xAxis()->setTitle(xTitle, "arialbd.ttf", 10, 0x000000);
    c->yAxis()->setTitle(yTitle, "arialbd.ttf", 10, 0x000000);
    c->yAxis()->setAutoScale(0.1, 0.1, 0.2);
    if (smooth) {
        SplineLayer *layer = c->addSplineLayer();
        layer->setXData(DoubleArray(xValues, nXSamples));
        layer->addDataSet(DoubleArray(yValues, nXSamples), 0x000000, "");
    }
    else {
        LineLayer *layer = c->addSplineLayer();
        layer->setXData(DoubleArray(xValues, nXSamples));
        layer->addDataSet(DoubleArray(yValues, nXSamples), 0x000000, "");
    }

    string svgName(chartTitle);
    string fullPath;

    svgName += string(".svg");
    fullPath = liveVar.projectDirectory  + svgName;
    writeMessageToParetoLog("Writing to file %s",fullPath.c_str());
    c->makeChart(fullPath.c_str());
    free(xValues);
    free(yValues);
    delete c;

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
void StructuralAnalysisDialog::onFEAFinished()
{
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::solveStructuralStatic()
{
    liveVar.zeroStrainTemperature = referenceTemperatureSpinBox->value();
    liveVar.nElemsToMesh = numElemsSpinBox->value();
    if (remeshCheckBox->isChecked()){
          coreFunctions.solveStatic();
    }
    else {
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
        topopt.solveElasticityStatic();
        topopt.postprocess();
        liveVar.STATIC_FEA_SOLVED = true;
        liveVar.displayGeom = DISPLAY_GEOM_MESH;
        liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;
      }
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void StructuralAnalysisDialog::nonlinearSolveStructural()
{
    int nNonlinearSteps, maxIter, nDeflationGroups, nElems;
    double tol;
    Timer timer;
    bool success;


    liveVar.nElemsToMesh = numElemsSpinBox->value();
    if (remeshCheckBox->isChecked()){
        coreFunctions.createTopOptMesh();
    }

    nElems = topopt.getNumElems();
    if (nElems == 0){
        writeMessageToGUIWindow("Error: Failed to create mesh");
        return;
    }

    resetProgressBar("Nonlinear FEA progress");

    nDeflationGroups  = int(nElems*1.0/500);
    nDeflationGroups = (nDeflationGroups < 500)? 500:nDeflationGroups;
    nDeflationGroups = (nDeflationGroups > 2000)? 2000:nDeflationGroups;
    topopt.enableDeflation(2,nDeflationGroups);

    liveVar.STATIC_FEA_SOLVED = false;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;

    bool forceTracking;

    forceTracking = false;

    forceData.clear();
    displacementData.clear();

    forceData.push_back(0);
    displacementData.push_back(0);

    nNonlinearSteps = liveVar.nonlinear.nSteps;
    maxIter = liveVar.nonlinear.nIterations;
    tol = liveVar.nonlinear.tolerance;

    //char inpFile[] = "LatticeMesh.inp";
    //topopt.dumpMeshAsAbaqusInpFile(inpFile);
    //return;
    timer.start();
    if (forceTracking)
        success = topopt.solveLargeDeformationForceTracking( nNonlinearSteps,  maxIter,  tol,nonlinearCallback);
    else { // displacement tracking
        success = topopt.solveLargeDeformationDisplacementTracking(nNonlinearSteps,  maxIter,  tol,nonlinearCallback);
    }

    timer.stop();
    if (!success)
       resetProgressBar("Nonlinear FEA unsuccessful");
    else
       resetProgressBar("Nonlinear FEA successful");

    topopt.postprocess();
    liveVar.timeForOneFEA = timer.getElapsedTimeInSec();

    liveVar.STATIC_FEA_SOLVED = true;
    writeMessageToGUIWindow("Pushing FEA results to geometry");
    topopt.pushInitialStaticSolutionToTessellation();
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;
    resetProgressBar("Nonlinear FEA complete");
    viewer->updateGL();

    char yLabel[] = "Force (N)";
    char xLabel[] = "Displacement (mm)";
    char title[256] = "FDCurve";// replaced below

    liveVar.modelName.copy(title,255,0);
    plotXY(displacementData,forceData, true, xLabel, yLabel, title);

}

/////////////////////////////////////////////////////
void StructuralAnalysisDialog::linearStructuralDynamics()
{
    int  nTimeSteps, nDeflationGroups, nElems, dof;
    Timer timer;
    bool success;
    double newmarkGamma, newmarkBeta, dampingM, dampingK, transientDeltaTime, transientTotalTime, tolerance;
    double trackPt[3],*timeData,*dispData;

    liveVar.nElemsToMesh = numElemsSpinBox->value();
    if (remeshCheckBox->isChecked()){
        coreFunctions.createTopOptMesh();
    }

    nElems = topopt.getNumElems();
    if (nElems == 0){
        writeMessageToGUIWindow("Error: Failed to create mesh");
        return;
    }


    nDeflationGroups  = int(nElems*1.0/500);
    nDeflationGroups = (nDeflationGroups < 500)? 500:nDeflationGroups;
    nDeflationGroups = (nDeflationGroups > 2000)? 2000:nDeflationGroups;
    topopt.enableDeflation(2,nDeflationGroups);

    topopt.initializeTransientElasticity();
    dampingM = liveVar.structuralDynamics.alphaM;
    dampingK = liveVar.structuralDynamics.betaK;
    newmarkGamma =liveVar.structuralDynamics.NewmarkGamma;
    newmarkBeta = liveVar.structuralDynamics.NewmarkGamma;
    transientTotalTime = liveVar.structuralDynamics.totalTime;
    transientDeltaTime = liveVar.structuralDynamics.timeStep;
    tolerance = 1e-10;// displacement has converged in time

    topopt.setTransientParams(newmarkGamma, newmarkBeta, dampingM, dampingK, transientDeltaTime, transientTotalTime, tolerance);

    trackPt[0] = liveVar.structuralDynamics.trackPoint[0];
    trackPt[1] = liveVar.structuralDynamics.trackPoint[1];
    trackPt[2] = liveVar.structuralDynamics.trackPoint[2];

    dof = liveVar.structuralDynamics.trackDOF;
    topopt.setTrackingOn(trackPt,dof);

    resetProgressBar("Linear Structural Dynamics in progress");
    topopt.solveElasticTransientCPUGPU(structuralDynamicsCallBack);
    resetProgressBar("Linear Structural Dynamics successful");

    // get the results and create svg file.
    nTimeSteps = topopt.getNumberOfTimeStepsTracked();
    timeData = (double *)malloc(nTimeSteps*sizeof(double));
    dispData = (double *)malloc(nTimeSteps*sizeof(double));
    topopt.getTimeStepsTracked(nTimeSteps,timeData);
    topopt.getDisplacementTracked(nTimeSteps,dispData);


    vector<double> timeVec;
    vector<double> dispVec;
    timeVec.clear();
    dispVec.clear();
    for (int i = 0; i < nTimeSteps; i++){
        timeVec.push_back(timeData[i]);
        dispVec.push_back(dispData[i]);
    }

    char yLabel[] = "Disp";
    char xLabel[] = "Time ";
    char title[256] = "Response";// replaced below

    liveVar.modelName.copy(title,255,0);
    plotXY(timeVec,dispVec, true, xLabel, yLabel, title);
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

    PushButton( staticStructuralSolvePushButton, [=]{ solveStructuralStatic();});
    PushButton( nonlinearStructuralSolvePushButton, [=]{ nonlinearSolveStructural();});
    PushButton( structuralDynamicsSolvePushButton, [=]{ linearStructuralDynamics();});
}
/////////////////////////////////////////////////////
