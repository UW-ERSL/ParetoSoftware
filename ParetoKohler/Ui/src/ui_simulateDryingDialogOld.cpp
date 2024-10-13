#include "ui_simulationDialog.hpp"
#include "gCodeReader.hpp"

extern gCodeReader             gCode;
extern void resetProgressBar(const char title[]);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);
bool moistureSimulationCallback(double fractionCompleted);

double *pseudoDensity;
int *elemInLine;
int previousLine;
double dryingTime;
bool continueSim;


int sampleCounter;
double ptA[3],ptB[3],ptC[3];
double boundingBox[3][2],centerX,centerY,radius;
bool solveElastic;


void collectData(double results[3])
{
    int nSamples;
    double avgA, avgB,avgC,humidity;
    avgA = 0;
    avgB = 0;
    avgC = 0;
    nSamples = 10;
    for (int i = 0; i < nSamples; i++){
        double theta;
        theta = 2*3.1415/nSamples;
        ptA[0] = centerX + radius*cos(theta);
        ptA[1] = centerY + radius*sin(theta);

        topopt.getTemperatureAt(ptA,&humidity);
        avgA += humidity;

        ptB[0] = centerX + radius*cos(theta);
        ptB[1] = centerY + radius*sin(theta);

        topopt.getTemperatureAt(ptB,&humidity);
        avgB += humidity;

        ptC[0] = centerX + radius*cos(theta);
        ptC[1] = centerY + radius*sin(theta);
        topopt.getTemperatureAt(ptC,&humidity);
        avgC += humidity;

    }
    results[0]  = avgA/ nSamples;
    results[1]  = avgB/ nSamples;
    results[2]  = avgC/ nSamples;
}
/************************************************************/
bool printingSimulationCallback(double fractionPrintCompleted)
{
    int nLines = gCode.getNumberOfGCodeLines();
    int numElemsInLine;
    double currentPrintTime,results[3],moisture,beta;
    char str[256];

    beta = 0.1;
    int currentLine = fractionPrintCompleted*nLines;
    for (int line = previousLine; line <= currentLine; line++){
        numElemsInLine = gCode.getElemsWithGCodeLineNumber(line,elemInLine);
        if (numElemsInLine <= 0) continue;
        for(int i = 0 ; i < numElemsInLine;i++){
            moisture =  topopt.getTemperatureInElem(elemInLine[i]);
            pseudoDensity[elemInLine[i]] = exp(-alpha*moisture);// bring back the element
            //pseudoDensity[elemInLine[i]] = 1.0;
        }
    }
    gCode.setNumberOfGCodeLinesToPlot(currentLine);
    topopt.setPseudoDensity(pseudoDensity);

    if (solveElastic){
        double acceleration[4]; // 3 linear and 1 radial
        acceleration[0] = acceleration[1] = acceleration[2] = acceleration[3] = 0;
        acceleration[2] = -9.81;
        topopt.setElasticityAcceleration(acceleration);
        //topopt.enableDeflation(DEFLATION_RIGID,500);
        topopt.enableSimpMode();
        topopt.solveElasticityStatic();
        topopt.disableDeflation();// issues with thermal deflation
    }
    topopt.applyConvectionOnAllExposedNodes();
    topopt.applyScaledConvectionOnZ0Surface(1500);
    previousLine = currentLine+1;
    currentPrintTime = fractionPrintCompleted*gCode.getGCodeTimeToPrint()/60;

    /*
    collectData(results);

    if (currentPrintTime < 60 ) // less than an hour
        sprintf(str,"Printing: %.1f (mins)\n A: %.1f \n B: %.1f \n C: %.1f ",
            currentPrintTime,results[0],results[1],results[2]);
    else{
        int hrs;
        double mins;
        hrs = currentPrintTime/60;
        mins = currentPrintTime - hrs*60;
        sprintf(str,"Printing: %d (hrs) %.0f (mins) \n A: %.1f \n B: %.1f \n C: %.1f ",hrs, mins,
                results[0],results[1],results[2]);
    }

    liveVar.graphicsText = string(str);
    */

    updateProgressBar("Print Simulation", 100.0*fractionPrintCompleted);
    return continueSim;
}
/************************************************************/
bool dryingSimulationCallback(double fractionDryingCompleted)
{
    double currentDryingTime,results[3];
    char str[256];

    updateProgressBar("Drying Simulation", 100.0*fractionDryingCompleted);
    currentDryingTime = fractionDryingCompleted*dryingTime/60;
    collectData(results);

    if (currentDryingTime < 60 ) // less than an hour
        sprintf(str,"Drying: %.1f (mins)\n A: %.1f \n B: %.1f \n C: %.1f ",
                currentDryingTime,results[0],results[1],results[2]);
    else{
        int hrs;
        double mins;
        hrs = currentDryingTime/60;
        mins = currentDryingTime - hrs*60;
        sprintf(str,"Dyring: %d (hrs) %.0f (mins) \n A: %.1f \n B: %.1f \n C: %.1f ",hrs, mins,
                results[0],results[1],results[2]);
    }
    liveVar.graphicsText = string(str);
    return continueSim;
}
/***************************************************/
MoistureDialog::MoistureDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    continueSim = true;

    vox.getBoundingBox(boundingBox);
    centerX = 0.5*boundingBox[0][0] + 0.5*boundingBox[0][1];
    centerY = 0.5*boundingBox[1][0] + 0.5*boundingBox[1][1];
    radius = 0.9*(boundingBox[0][1] - boundingBox[0][0])/2;

    ptA[2] = 0.1*boundingBox[2][0] + 0.9*boundingBox[2][1];
    ptB[2] = 0.6*boundingBox[2][0] + 0.4*boundingBox[2][1];
    ptC[2] = 0.9*boundingBox[2][0] + 0.1*boundingBox[2][1];
}
///////////////////////////////////////////////////////////////////////////////
void MoistureDialog :: simulatePrinting()
{
    double deltaTime,initialMoisture, relativeHumidity;
    double hPrint,density;

    int simulationSteps;
    double printTime;

    previousLine  = 0;
    int nElems = topopt.getNumElems();
    if (nElems == 0) return;
    continueSim = true;
    propertyLineEdit[0]->setValidator( new QDoubleValidator(this) );
    initialMoisture = propertyLineEdit[0]->text().toDouble();

    propertyLineEdit[1]->setValidator( new QDoubleValidator(this) );
    relativeHumidity = propertyLineEdit[1]->text().toDouble();

    propertyLineEdit[2]->setValidator( new QDoubleValidator(this) );
    hPrint = 1e-4*propertyLineEdit[2]->text().toDouble();

    propertyLineEdit[3]->setValidator( new QDoubleValidator(this) );
    density = propertyLineEdit[5]->text().toDouble();

    propertyLineEdit[5]->setValidator( new QDoubleValidator(this) );
    simulationSteps = propertyLineEdit[5]->text().toInt();// hrs into seconds

    elemInLine = (int *)malloc(topopt.getNumElems()*sizeof(int));

    solveElastic = false;// don't solve elastic after thermal
    pseudoDensity = (double *)malloc(nElems*sizeof(double));
    for (int i =0; i < nElems; i++)
        pseudoDensity[i] = 0.0;

    topopt.setPseudoDensity(pseudoDensity);
    topopt.setThermoElasticReferenceTemperature(17.5); // heuristic that works well for printing
    topopt.setConvectionCoefficient(hPrint);

    printTime = gCode.getGCodeTimeToPrint();
    deltaTime = printTime/simulationSteps;

    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_MESH;
    liveVar.displayField = DISPLAY_FIELD_MOISTURE;

    if (includeGravityCheckBox->isChecked()){
        topopt.fixNodesOnZ0Surface();
        liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;
        liveVar.STATIC_FEA_SOLVED = true;
        solveElastic = true;
    }

    resetProgressBar("Printing Simulation");
    topopt.disableDeflation();// issues with thermal deflation

    topopt.solveTransientThermoElasticProblem(initialMoisture, NULL, deltaTime, printTime, false, printingSimulationCallback);

    free(elemInLine);
    free(pseudoDensity);

    viewer->updateGL();
    resetProgressBar("Print Simulation Done");
}
///////////////////////////////////////////////////////////////////////////////
void MoistureDialog :: simulateDrying()
{
    double initialMoisture,deltaTime,*initialMoistureDistribution,hDry,relativeHumidity;
    bool solveElastic;
    int simulationSteps;

    int nElems = topopt.getNumElems();
    if (nElems == 0) return;
    continueSim = true;
    propertyLineEdit[0]->setValidator( new QDoubleValidator(this) );
    initialMoisture = propertyLineEdit[0]->text().toDouble();

    propertyLineEdit[1]->setValidator( new QDoubleValidator(this) );
    relativeHumidity = propertyLineEdit[1]->text().toDouble();

    propertyLineEdit[3]->setValidator( new QDoubleValidator(this) );
    hDry = 1e-4*propertyLineEdit[3]->text().toDouble();

    propertyLineEdit[4]->setValidator( new QDoubleValidator(this) );
    dryingTime = 3600*propertyLineEdit[4]->text().toDouble();// hrs into seconds

    propertyLineEdit[6]->setValidator( new QDoubleValidator(this) );
    simulationSteps = propertyLineEdit[6]->text().toInt();// hrs into seconds


    //topopt.setConvectionCoefficient(hDry);

    //topopt.applyConvectionOnAllExposedNodes();
    //topopt.applyScaledConvectionOnZ0Surface(20);
    int nNodes = topopt.getNumNodes();
    initialMoistureDistribution = (double *)malloc(nNodes*sizeof(double));

    topopt.getTemperatureInAllNodes(initialMoistureDistribution);
    deltaTime = dryingTime/simulationSteps;
    solveElastic = false;// don't solve elastic after thermal
    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_MESH;
    liveVar.displayField = DISPLAY_FIELD_MOISTURE;
    resetProgressBar("Drying Simulation");
    // ste up sample points
    sampleCounter = 0;


    topopt.solveTransientThermoElasticProblem(initialMoisture, initialMoistureDistribution, deltaTime, dryingTime, solveElastic, dryingSimulationCallback);
    viewer->updateGL();
    resetProgressBar("Drying Simulation Done");
    liveVar.THERMAL_FEA_SOLVED = true;
    emit setMoistureComplete();
    free(initialMoistureDistribution);

}
///////////////////////////////////////////////////////////////////////////////
void MoistureDialog :: stopSimulation()
{
    continueSim = false;
}
///////////////////////////////////////////////////////////////////////////////
void MoistureDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void MoistureDialog :: makeConnections()
{
    PushButton( simulatePrintingPushButton, [=]{simulatePrinting(); });
    PushButton( simulateDryingPushButton, [=]{simulateDrying(); });
    PushButton( stopPushButton, [=]{stopSimulation(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}
/***************************************************/
MoistureDialog::~MoistureDialog()
{
    ;
}
