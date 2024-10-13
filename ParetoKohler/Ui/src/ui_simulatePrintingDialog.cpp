#include "ui_simulatePrintingDialog.hpp"
#include "gCodeReader.hpp"

extern gCodeReader             gCode;
extern void resetProgressBar(const char title[]);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);
bool moistureSimulationCallback(double fractionCompleted);

double *pseudoDensity;
int *elemInLine;
int previousLine;
bool continueSim;


int sampleCounter;
double ptA[3],ptB[3],ptB1[3],ptB2[3],ptC[3];
double boundingBox[3][2],centerX,centerY,radius;
bool solveElastic;
double beta;


void collectData(double results[5])
{
    int nSamples;
    double avgA, avgB,avgC,avgB1, avgB2, humidity;
    avgA = 0;
    avgB = 0;
    avgB1 = 0;
    avgB2 = 0;
    avgC = 0;


    nSamples = 10;
    for (int i = 0; i < nSamples; i++){
        double theta;
        theta = 2*3.1415/nSamples;
        ptA[0] = centerX + radius*cos(theta);
        ptA[1] = centerY + radius*sin(theta);
        topopt.getTemperatureAt(ptA,&humidity);
        avgA += humidity;

        ptB1[0] = centerX + radius*cos(theta);
        ptB1[1] = centerY + radius*sin(theta);
        topopt.getTemperatureAt(ptB1,&humidity);
        avgB1 += humidity;

        ptB[0] = centerX + radius*cos(theta);
        ptB[1] = centerY + radius*sin(theta);
        topopt.getTemperatureAt(ptB,&humidity);
        avgB += humidity;


        ptB2[0] = centerX + radius*cos(theta);
        ptB2[1] = centerY + radius*sin(theta);
        topopt.getTemperatureAt(ptB2,&humidity);
        avgB2 += humidity;


        ptC[0] = centerX + radius*cos(theta);
        ptC[1] = centerY + radius*sin(theta);
        topopt.getTemperatureAt(ptC,&humidity);
        avgC += humidity;

    }
    results[0]  = avgA/ nSamples;
    results[1]  = avgB1/ nSamples;
    results[2]  = avgB/ nSamples;
    results[3]  = avgB2/ nSamples;
    results[4]  = avgC/ nSamples;
}
/************************************************************/
bool printingSimulationCallback(double fractionPrintCompleted)
{
    int nLines = gCode.getNumberOfGCodeLines();
    int numElemsInLine;
    double currentPrintTime,moisture,beta;

    int currentLine = fractionPrintCompleted*nLines;
    for (int line = previousLine; line <= currentLine; line++){
        numElemsInLine = gCode.getElemsWithGCodeLineNumber(line,elemInLine);
        if (numElemsInLine <= 0) continue;
        for(int i = 0 ; i < numElemsInLine;i++){
            moisture =  topopt.getTemperatureInElem(elemInLine[i]);
            //pseudoDensity[elemInLine[i]] = exp(-beta*moisture);// bring back the element
            pseudoDensity[elemInLine[i]] = 1.0;
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

        topopt.solveElasticityStatic();
        topopt.disableDeflation();// issues with thermal deflation
    }
    topopt.applyConvectionOnAllExposedNodes();
    topopt.applyScaledConvectionOnZ0Surface(1500);
    previousLine = currentLine+1;
    currentPrintTime = fractionPrintCompleted*gCode.getGCodeTimeToPrint()/60;


    double results[5];
    char str[256];
    collectData(results);

    if (currentPrintTime < 60 ) // less than an hour
        sprintf(str,"Printing: %.1f (mins)\n A: %.1f \n B1: %.1f  B: %.1f B2: %.1f C: %.1f ",
            currentPrintTime,results[0],results[1],results[2],results[3],results[4]);
    else{
        int hrs;
        double mins;
        hrs = currentPrintTime/60;
        mins = currentPrintTime - hrs*60;
        sprintf(str,"Printing: %d (hrs) %.0f (mins) \n A: %.1f \n B1: %.1f  B: %.1f B2: %.1f C: %.1f ",hrs, mins,
                results[0],results[1],results[2],results[3],results[4]);
    }

    liveVar.graphicsText = string(str);


    updateProgressBar("Print Simulation", 100.0*fractionPrintCompleted);
    return continueSim;
}

/***************************************************/
PrintingDialog::PrintingDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    continueSim = true;

    vox.getBoundingBox(boundingBox);
    centerX = 0.5*boundingBox[0][0] + 0.5*boundingBox[0][1];
    centerY = 0.5*boundingBox[1][0] + 0.5*boundingBox[1][1];
    radius = 0.9*(boundingBox[0][1] - boundingBox[0][0])/2;

    ptA[2] = 0.1*boundingBox[2][0] + 0.9*boundingBox[2][1];
    ptB1[2] = 0.25*boundingBox[2][0] + 0.75*boundingBox[2][1];
    ptB[2] = 0.6*boundingBox[2][0] + 0.4*boundingBox[2][1];
    ptB2[2] = 0.75*boundingBox[2][0] + 0.25*boundingBox[2][1];
    ptC[2] = 0.9*boundingBox[2][0] + 0.1*boundingBox[2][1];
}
///////////////////////////////////////////////////////////////////////////////
void PrintingDialog :: simulatePrinting()
{
    double deltaTime,initialMoisture, relativeHumidity;
    double hPrint;

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
    hPrint = 1e-5*propertyLineEdit[2]->text().toDouble();

    propertyLineEdit[3]->setValidator( new QDoubleValidator(this) );
    beta = propertyLineEdit[3]->text().toDouble();

    propertyLineEdit[4]->setValidator( new QDoubleValidator(this) );
    simulationSteps = propertyLineEdit[4]->text().toInt();// hrs into seconds


    elemInLine = (int *)malloc(topopt.getNumElems()*sizeof(int));

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

    if (solveElastic){
        liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;
        liveVar.STATIC_FEA_SOLVED = true;
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
void PrintingDialog :: simulateMoistureAndGravity()
{
    topopt.fixNodesOnZ0Surface();
    solveElastic = true;
    topopt.enableSimpMode();
    topopt.setEexponent(1.0);
    simulatePrinting();
}///////////////////////////////////////////////////////////////////////////////
void PrintingDialog :: simulateMoisture()
{
    solveElastic = false;
    simulatePrinting();
}
///////////////////////////////////////////////////////////////////////////////
void PrintingDialog :: stopSimulation()
{
    continueSim = false;
}
///////////////////////////////////////////////////////////////////////////////
void PrintingDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void PrintingDialog :: makeConnections()
{
    PushButton( simulateMoisturePushButton, [=]{simulateMoisture(); });
    PushButton( simulateMoistureAndGravityPushButton, [=]{simulateMoistureAndGravity(); });
    PushButton( stopPushButton, [=]{stopSimulation(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}
/***************************************************/
PrintingDialog::~PrintingDialog()
{
    ;
}
