#include "ui_meshDialog.hpp"
#include "CoreFunctions.hpp"
#include "gCodeReader.hpp"

extern gCodeReader             gCode;
extern void resetProgressBar(const char title[]);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);
/***************************************************/
MeshDialog::MeshDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    double layerHeight,volume;
    int numElemsRecommended;

    layerHeight = gCode.getGCodeLayerHeight(); // in meters
    volume = vox.getVolumeOfSTLPart(0);// in m^3
    numElemsRecommended = NEARESTINT(volume/(pow(layerHeight,3.0)));
    numElemsRecommended = numElemsRecommended < 10000000? numElemsRecommended: 10000000;
    numElemsRecommended = numElemsRecommended < 50000? 50000:numElemsRecommended;

    propertyLineEdit[3]->setText(QString::number(numElemsRecommended));

    pausedSimulation = false;
    continueSimulation = true;
}
///////////////////////////////////////////////////////////////////////////////
void MeshDialog :: createMesh()
{
    int numElements;
    double E,density,nu,conductivity,alpha,specificHeat,diffusivity;

    propertyLineEdit[0]->setValidator( new QDoubleValidator(this) );
    E = 1e6*propertyLineEdit[0]->text().toDouble();//convert from MPa

    propertyLineEdit[1]->setValidator( new QDoubleValidator(this) );
    density = propertyLineEdit[1]->text().toDouble();

    propertyLineEdit[2]->setValidator( new QDoubleValidator(this) );
    diffusivity = 1e-7*propertyLineEdit[2]->text().toDouble();

    // the number of elements recommended depends on the layer height and volume of part
    propertyLineEdit[3]->setValidator(new QIntValidator(this) );
    numElements = propertyLineEdit[3]->text().toInt();

    // Solve the moisture problem by modeling it as a transient thermal problem
    // Internally, the thermal problem is solved by using the thermal conductivity, etc
    // In thermal problems, diffusivity is defined as
    // D = K/(rho*Cp)
    // However, here only the diffusivity matters.

    nu = 0.2;
    conductivity = diffusivity*density;
    specificHeat = 1;
    alpha = 0;

    topopt.setIsotropicMaterialOfPart(0, E, nu,density,conductivity,alpha,specificHeat, "Clay");

    liveVar.nElemsToMesh = numElements;
    coreFunctions.createTopOptMesh();

    int nElems = topopt.getNumElems();
    if (nElems = 0) return;

    liveVar.displayGeom = DISPLAY_GEOM_MESH;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    //liveVar.geomTransparent = true;

    resetProgressBar("Meshing done");
    liveVar.MESH_APPLIED = true;
    viewer->updateGL();
    emit setMeshComplete();
}
///////////////////////////////////////////////////////////////////////////////
void MeshDialog :: simulateMesh()
{
    int startLine;
    double *pseudoDensity;

    int nElems = topopt.getNumElems();
    if (nElems == 0) return;

    int nLines = gCode.getNumberOfGCodeLines();
    pseudoDensity = (double *)malloc(nElems*sizeof(double));
    for (int i =0; i < nElems; i++)
        pseudoDensity[i] =0;

    topopt.setPseudoDensity(pseudoDensity);
    int numElemsInLine;
    int *elemInLine;
    elemInLine = (int *)malloc(topopt.getNumElems()*sizeof(int));
    liveVar.displayGeom = DISPLAY_GEOM_MESH;
    liveVar.geomTransparent = true;
    viewer->updateGL();
    resetProgressBar("Mesh Simulation");
    startLine = 0;
    startOfMeshSimulation:
    while (pausedSimulation){
        updateProgressBar("Mesh Simulation", 100.0*startLine/nLines);
        viewer->updateGL();
        continue;
    }
    for(int line = startLine; line < nLines;line++){
        numElemsInLine = gCode.getElemsWithGCodeLineNumber(line,elemInLine);
        if (numElemsInLine <= 0) continue;
        for(int i = 0 ; i < numElemsInLine;i++)
            pseudoDensity[elemInLine[i]] = 1.0; // bring back the element

        topopt.setPseudoDensity(pseudoDensity);
        //if (line < 500) cout << line << "  " << topopt.countNumElemsIn() << endl;
        if (line%100 != 0) continue; // avoids redrawing for faster simulation
        updateProgressBar("Mesh Simulation", 100.0*line/nLines);
        viewer->updateGL();
        if (pausedSimulation) {
            startLine = line+1;
            goto startOfMeshSimulation;
        }
        if (!continueSimulation) break;
    }
    free(elemInLine);
    free(pseudoDensity);
    resetProgressBar("Mesh Simulation Done");
    continueSimulation = true;
    pausedSimulation = false;
}
///////////////////////////////////////////////////////////////////////////////
void MeshDialog :: pauseSimulation()
{
    if (!pausedSimulation){
        pausePushButton->setText(QApplication::translate("MeshDialog", "Continue Simulation", Q_NULLPTR));
        pausedSimulation = true;
    }
    else{
        pausePushButton->setText(QApplication::translate("MeshDialog", "Pause Simulation", Q_NULLPTR));
        pausedSimulation = false;
    }
}
///////////////////////////////////////////////////////////////////////////////
void MeshDialog :: stopSimulation()
{
    continueSimulation = false;
    pausePushButton->setText(QApplication::translate("MeshDialog", "Pause Simulation", Q_NULLPTR));
    pausedSimulation = false;
}
///////////////////////////////////////////////////////////////////////////////
void MeshDialog :: closeDialog()
{

    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void MeshDialog :: makeConnections()
{
    PushButton( createMeshPushButton, [=]{createMesh(); });
    PushButton( simulateMeshPushButton, [=]{simulateMesh(); });
    PushButton( pausePushButton, [=]{pauseSimulation(); });
    PushButton( stopPushButton, [=]{stopSimulation(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}
/***************************************************/
MeshDialog::~MeshDialog()
{
    ;
}
