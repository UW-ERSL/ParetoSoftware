#include <QtGui>
#include <iostream>
#include<QApplication>
//using namespace std;
#include <QMainWindow>
#include "ui_mainWindow.hpp"
#include "CoreFunctions.hpp"
#include "Renderer.hpp"
#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "LiveVariables.hpp"
#include "MaterialHandler.hpp"


ParetoApplication       topopt;
ParetoApplication       topoptTemp;//used in certain applications (ex: lattice structure design)
Voxelizer               vox;
Voxelizer               voxTemp;// used in certain applications
LiveVariables           liveVar;
CoreFunctions           coreFunctions;
Renderer                renderer;
MaterialHandler         materialHandler;


#ifdef COMPILE_GPUCODE
#pragma comment(lib, "cuda.lib")
#pragma comment(lib, "cusparse.lib")
#pragma comment(lib, "cublas.lib")
#endif

int main(int argc, char** argv)
{
    QApplication application(argc,argv);
    TopOptMainWindow  win;


    std::cout.precision(4);
    coreFunctions.oneTimeInitialization();
    coreFunctions.initializeLiveVariables();
    coreFunctions.resetLiveVariables();

    liveVar.usingGUI = true;
    QFile MaterialFile(":/resources/ParetoMaterials.csv");
    MaterialFile.copy(qApp->applicationDirPath().append("/ParetoMaterials.csv"));
#ifdef _WIN32 // resource file does not load in Mac!!
    materialHandler.readMaterialCSV("./ParetoMaterials.csv");
#endif
    win.setWindowTitle("Pareto");
    win.show();
    return application.exec();

}
