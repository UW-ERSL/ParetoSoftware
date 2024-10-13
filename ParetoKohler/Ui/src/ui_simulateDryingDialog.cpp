#include "ui_simulateDryingDialog.hpp"
#include "gCodeReader.hpp"

extern gCodeReader             gCode;
extern void resetProgressBar(const char title[]);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);
bool moistureSimulationCallback(double fractionCompleted);

bool continueDryingSim;
extern double *pseudoDensity;
extern int *elemInLine;
extern int previousLine;
double dryingTime;

extern int sampleCounter;
extern double ptA[3],ptB[3],ptC[3];
extern double boundingBox[3][2],centerX,centerY,radius;
extern bool solveElastic;

extern void collectData(double results[3]);

/************************************************************/
bool dryingSimulationCallback(double fractionDryingCompleted)
{
    double currentDryingTime,results[5];
    char str[256];

    updateProgressBar("Drying Simulation", 100.0*fractionDryingCompleted);
    currentDryingTime = fractionDryingCompleted*dryingTime/60;
    collectData(results);

    if (currentDryingTime < 60 ) // less than an hour
        sprintf(str,"Printing: %.1f (mins)\n A: %.1f \n B1: %.1f  B: %.1f B2: %.1f C: %.1f ",
                currentDryingTime,results[0],results[1],results[2],results[3],results[4]);
    else{
        int hrs;
        double mins;
        hrs = currentDryingTime/60;
        mins = currentDryingTime - hrs*60;
        sprintf(str,"Printing: %d (hrs) %.0f (mins) \n A: %.1f \n B1: %.1f  B: %.1f B2: %.1f C: %.1f ",hrs, mins,
                results[0],results[1],results[2],results[3],results[4]);
    }
    liveVar.graphicsText = string(str);
    return continueDryingSim;
}
/***************************************************/
DryingDialog::DryingDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    continueDryingSim = true;

    vox.getBoundingBox(boundingBox);
    centerX = 0.5*boundingBox[0][0] + 0.5*boundingBox[0][1];
    centerY = 0.5*boundingBox[1][0] + 0.5*boundingBox[1][1];
    radius = 0.9*(boundingBox[0][1] - boundingBox[0][0])/2;

    ptA[2] = 0.1*boundingBox[2][0] + 0.9*boundingBox[2][1];
    ptB[2] = 0.6*boundingBox[2][0] + 0.4*boundingBox[2][1];
    ptC[2] = 0.9*boundingBox[2][0] + 0.1*boundingBox[2][1];
}
///////////////////////////////////////////////////////////////////////////////
void DryingDialog :: simulateDrying()
{
    double deltaTime,*initialMoistureDistribution,hDry,initialMoisture;
    bool solveElastic;
    int dryingSteps;

    int nElems = topopt.getNumElems();
    if (nElems == 0) return;
    continueDryingSim = true;

    propertyLineEdit[0]->setValidator( new QDoubleValidator(this) );
    hDry = 1e-5*propertyLineEdit[0]->text().toDouble();

    propertyLineEdit[1]->setValidator( new QDoubleValidator(this) );
    dryingTime = 3600*propertyLineEdit[1]->text().toDouble();// hrs into seconds

    propertyLineEdit[2]->setValidator( new QDoubleValidator(this) );
    dryingSteps = propertyLineEdit[2]->text().toInt();


    topopt.setConvectionCoefficient(hDry);
    //topopt.setThermoElasticReferenceTemperature(17);
    //topopt.applyConvectionOnAllExposedNodes();
    //topopt.applyScaledConvectionOnZ0Surface(20);
    int nNodes = topopt.getNumNodes();
    initialMoistureDistribution = (double *)malloc(nNodes*sizeof(double));

    topopt.getTemperatureInAllNodes(initialMoistureDistribution);
    deltaTime = dryingTime/dryingSteps;
    solveElastic = false;// don't solve elastic after thermal
    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_MESH;
    liveVar.displayField = DISPLAY_FIELD_MOISTURE;
    resetProgressBar("Drying Simulation");
    // ste up sample points
    sampleCounter = 0;
    initialMoisture = 0;// not relevant

    topopt.solveTransientThermoElasticProblem(initialMoisture, initialMoistureDistribution, deltaTime, dryingTime, solveElastic, dryingSimulationCallback);
    viewer->updateGL();
    resetProgressBar("Drying Simulation Done");
    liveVar.THERMAL_FEA_SOLVED = true;
    emit setMoistureComplete();
    free(initialMoistureDistribution);

}
///////////////////////////////////////////////////////////////////////////////
void DryingDialog :: stopSimulation()
{
    continueDryingSim = false;
}
///////////////////////////////////////////////////////////////////////////////
void DryingDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void DryingDialog :: makeConnections()
{
    PushButton( simulateDryingPushButton, [=]{simulateDrying(); });
    PushButton( stopPushButton, [=]{stopSimulation(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}
/***************************************************/
DryingDialog::~DryingDialog()
{
    ;
}
