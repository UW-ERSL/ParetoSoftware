#include <QtGui>
#include <iostream>
#include<QApplication>
//using namespace std;
#include <QMainWindow>
#include "ui_mainWindow.hpp"
#include "LatticeRenderer.hpp"
#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "LiveVariables.hpp"
#include "MaterialHandler.hpp"


#include "../ParetoTopOpt/src/CoreFunctions.cpp"
#include "../ParetoTopOpt/src/text3d.cpp"
#include "../ParetoTopOpt/src/MaterialHandler.cpp"


ParetoApplication       topopt;
ParetoApplication       topoptTemp;//used in certain applications (ex: lattice structure design)
Voxelizer               vox;
Voxelizer               voxTemp;
LiveVariables           liveVar;
CoreFunctions           coreFunctions;
Renderer                renderer;// standard renderer
TemplateRenderer        templateRenderer;// additional renderer
MaterialHandler         materialHandler;
LatticeParameters       tpmsParams;

LatticeParameters latticeParams;

int main(int argc, char** argv)
{
    QApplication application(argc,argv);
    TopOptMainWindow  win;

    std::cout.precision(4);
    coreFunctions.oneTimeInitialization();
    coreFunctions.initializeLiveVariables();
    coreFunctions.resetLiveVariables();

    QFile MaterialFile(":/resources/ParetoMaterials.csv");
    MaterialFile.copy(qApp->applicationDirPath().append("/ParetoMaterials.csv"));
    materialHandler.readMaterialCSV("./ParetoMaterials.csv");

    win.setWindowTitle("ParetoLattice");
    win.show();
    return application.exec();

}


