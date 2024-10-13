#include "ui_gCodeDialog.hpp"
#include "gCodeReader.hpp"
#include "Timer.h"
extern gCodeReader             gCode;
extern void resetProgressBar(const char title[]);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);
extern void writeWarningToGUIWindow(const char format[],...);
extern Voxelizer vox;
Timer timer;
/***************************************************/
GCodeDialog::GCodeDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();

    pausedSimulation = false;
    continueSimulation = true;
}
/***************************************************/
void GCodeDialog :: loadGCode()
{
    double layerHeight, layerWidth;
    QString qstr = QFileDialog::getOpenFileName(this,*new QString("Select GCode File "),
                                                QString(""), *new QString( "GCode Format (*.gcode)"));
    string fileName = qstr.toUtf8().constData();
    if (fileName.empty())
        return;
    if ( (fileName.find(".gcode") == std::string::npos)){
        writeWarningToLog("File format not recognized; acceptable extensions: .gcode");
        return;
    }
    gCode.readGCode(fileName);
    gCode.scaleGCode(0.001); // mm to meters

    double boundingBoxSTL[3][2],boundingBoxGCode[3][2],gCodeCenter[3],offset[3],stlCenter[3];

    vox.getBoundingBoxOfPart(0,boundingBoxSTL);
    for (int i = 0; i < 3; i++)
        stlCenter[i] = (boundingBoxSTL[i][0] + boundingBoxSTL[i][1])/2;
    gCode.getBoundingBoxFromGCode(boundingBoxGCode,gCodeCenter);
    for (int i = 0; i < 3; i++)
        offset[i] = stlCenter[i] - gCodeCenter[i];
    gCode.shiftGCodeby( offset);
    //gCode.shiftGCodeToOrigin();


    //double offset[3];
    //offset[0] = -0.02; offset[1] = -0.008; offset[2] = 0;
    //gCode.shiftGCodeby( offset);

    //layerHeight = gCode.getGCodeLayerHeight();

    //propertyLineEdit[0]->setText(QString::number(layerHeight*1000));
    layerHeight = 0.001*propertyLineEdit[0]->text().toDouble();// convert to meters

    gCode.setGCodeLayerHeight(layerHeight);

    int nLines  = gCode.getNumberOfGCodeLines();
    gCode.setNumberOfGCodeLinesToPlot(nLines);
    double totalTime = gCode.getGCodeTimeToPrint();

    if (totalTime < 0.01){  // some gcode generators such as SLICR don't have explicit time information
        writeWarningToGUIWindow("%s","Explicit time information missing in gcode.");
        liveVar.GCODE_LOADED = false;
    }
    else{
       liveVar.GCODE_LOADED = true;
       emit gCodeComplete();
    }
    liveVar.plotAxis = false;
    liveVar.displayGeom = DISPLAY_GEOM_GCODE;
    liveVar.geomTransparent = true;
    viewer->updateGL();

}
/***************************************************/
void GCodeDialog :: simulateGCode()
{
    double layerHeight,layerWidth,volume;
    int startLine;

    propertyLineEdit[0]->setValidator( new QDoubleValidator(this) );// comes in as mm
    layerHeight = 0.001*propertyLineEdit[0]->text().toDouble();// convert to meters

    layerWidth = 2*layerHeight;
    gCode.setGCodeLayerHeight(layerHeight);
    gCode.setGCodeLayerWidth(layerWidth);
    gCode.printGCodeInformation();
    volume = vox.getVolumeOfSTLPart(0);

    gCode.setNumberOfGCodeLinesToPlot(0);
    liveVar.displayGeom = DISPLAY_GEOM_GCODE_FILAMENT;
    liveVar.plotAxis = false;
    liveVar.geomTransparent = true;
    viewer->updateGL();
    int nLines  = gCode.getNumberOfGCodeLines();
    startLine = 0;
    startOfGCodeSimulation:
    while (pausedSimulation){
        updateProgressBar("GCode Simulation", 100.0*startLine/nLines);
        viewer->updateGL();
        continue;
    }
    for(int line = startLine; line < nLines;line++){
        updateProgressBar("GCode Simulation", 100.0*line/nLines);
        gCode.setNumberOfGCodeLinesToPlot(line);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        viewer->updateGL();
        if (pausedSimulation) {
            startLine = line+1;
            goto startOfGCodeSimulation;
        }
        if (!continueSimulation) break;
        //timer.pause(25);
    }
    resetProgressBar("");
    continueSimulation = true;
    pausedSimulation = false;
}
///////////////////////////////////////////////////////////////////////////////
void GCodeDialog :: pauseSimulation()
{
    if (!pausedSimulation){
        pauseGCodePushButton->setText(QApplication::translate("GCodeDialog", "Continue Simulation", Q_NULLPTR));
        pausedSimulation = true;
    }
    else{
        pauseGCodePushButton->setText(QApplication::translate("GCodeDialog", "Pause Simulation", Q_NULLPTR));
        pausedSimulation = false;
    }
}
///////////////////////////////////////////////////////////////////////////////
void GCodeDialog :: stopSimulation()
{
    continueSimulation = false;

    pauseGCodePushButton->setText(QApplication::translate("GCodeDialog", "Pause Simulation", Q_NULLPTR));
    pausedSimulation = false;
}
///////////////////////////////////////////////////////////////////////////////
void GCodeDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void GCodeDialog :: makeConnections()
{
    PushButton( loadGCodePushButton, [=]{loadGCode(); });
    PushButton( simulateGCodePushButton, [=]{simulateGCode(); });
    PushButton( pauseGCodePushButton, [=]{pauseSimulation(); });
    PushButton( stopGCodePushButton, [=]{stopSimulation(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}
/***************************************************/
GCodeDialog::~GCodeDialog()
{
    ;
}
